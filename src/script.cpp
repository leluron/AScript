#include <ascript/script.h>
#include <istream>
#include <vector>

#include <antlr4-runtime/antlr4-runtime.h>
#include "parser/ASParser.h"
#include "parser/ASLexer.h"

using namespace std;

// Extract AST from antlr context
statp toAST(ASParser::FileContext *file); 

// Load AST from file
void Script::load(string path) {
    ifstream stream(path);
    antlr4::ANTLRInputStream input(stream);
    ASLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ASParser parser(&tokens);
    ASParser::FileContext* tree = parser.file();

    stringstream ss;
    stream.seekg(0);
    ss << stream.rdbuf();
    this->source = ss.str();
    this->filename = path;
    code = toAST(tree);
}

Script::Script(string path) {
    variables->getRef("assert") = valp(new ValueNativeFunc([](auto a) {
        if (!a[0]->isTrue()) throw runtime_error("Assertion failed");
        return valp(new ValueNone());
    }));
    load(path);
}

void Script::exec(valp vars,statp sp) {
    try {
        if (auto s = dynamic_pointer_cast<AssignStat>(sp)) {
            // eval right side
            auto r = eval(vars, s->right);
            // get reference to left side
            auto& v = evalRef(vars, s->left);
            // Specialization for extern values
            if (auto vv = dynamic_pointer_cast<ValueExternBase>(v)) {
                vv->assign(r);
            } else {
                // if no extern just point to right side value
                v = r;
            }
        }
        else if (auto s = dynamic_pointer_cast<CompAssignStat>(sp)) {
            // eval right side
            auto r = eval(vars, s->right);
            // get reference to left side
            auto& v = evalRef(vars, s->left);
            string op(1, s->op[0]);
            v = v->binop(op, r);
        }
        else if (auto s = dynamic_pointer_cast<FuncCallStat>(sp)) {
            expp fce = expp(new FuncCallExp(s->ctx, s->f, s->a));
            fce->srcinfo = sp->srcinfo;
            eval(vars, fce);
        }
        else if (auto s = dynamic_pointer_cast<IfStat>(sp)) {
            auto vi = eval(vars, s->cond);
            if (vi->isTrue()) exec(vars, s->then);
            else exec(vars, s->els);
        }
        else if (auto s = dynamic_pointer_cast<BlockStat>(sp)) {
            for (auto ss : s->stats) {
                exec(vars, ss);
                // stop block if return stat executed
                if (ret) return;
            }
        }
        else if (auto s = dynamic_pointer_cast<WhileStat>(sp)) {
            while (true) {
                auto vi = eval(vars, s->cond);
                if (vi->isTrue()) {
                    exec(vars, s->stat);
                    // stop loop if return stat executed
                    if (ret) return;
                }
                else break;
            }
        }
        else if (auto s = dynamic_pointer_cast<ForStat>(sp)) {
            auto list = eval(vars, s->list);
            for (int i=0;i<list->length();i++) {
                vars->getRef(s->id) = list->at(i);
                exec(vars, s->stat);
                // stop loop if return stat executed
                if (ret) return;
            }
        }
        else if (auto s = dynamic_pointer_cast<ReturnStat>(sp)) {
            if (s->e) {
                ret = eval(vars, s->e);
            } else {
                ret = valp(new ValueNone());
            }
        }
        else throw runtime_error("Unknown statement");
    } catch (runtime_error e) {
        throw InterpreterError(filename, source, sp->srcinfo, e.what());
    }
}

valp Script::evalFunc(valp ctx, string fn, vector<valp> args) {
    // extract function or method
    auto f0 = ctx->getRef(fn);
    if (auto f = dynamic_pointer_cast<ValueFunction>(f0)) {
        // In case of script function
        // Check argument number
        if (f->args.size() != args.size()) throw runtime_error("Unmatching arguments");
        // place arguments in a map associated with argument names
        valp env = valp(new ValueMap({}));
        for (int i=0;i<f->args.size();i++) {
            auto argName = f->args[i];
            if (argName == "this") throw runtime_error("Argument can't be named `this`");
            env->getRef(argName) = args[i];
        }
        // link `this`
        env->getRef("this") = ctx;
        // run function
        exec(env, f->body);
        // extract return value
        auto v = ret;
        if (!v) v = valp(new ValueNone());
        // as we come back to the underlying code reset return indicator
        ret = nullptr;
        return v;
    } else if (auto f = dynamic_pointer_cast<ValueNativeFunc>(f0)) {
        // in case of native function
        // run function and get return value
        return f->f(args);
    } else throw runtime_error("Can't call non-function");
}

valp Script::eval1(valp vars, expp ep) {
     if (auto e = dynamic_pointer_cast<IntExp>(ep)) {
        return valp(new ValueInt(e->value));
    }
    else if (auto e = dynamic_pointer_cast<FloatExp>(ep)) {
        return valp(new ValueFloat(e->value));
    }
    else if (auto e = dynamic_pointer_cast<IdExp>(ep)) {
        return vars->getRef(e->name);
    }
    else if (auto e = dynamic_pointer_cast<BinOpExp>(ep)) {
        auto v1 = eval(vars, e->l);
        auto v2 = eval(vars, e->r);
        return v1->binop(e->op, v2);
    }
    else if (auto e = dynamic_pointer_cast<UnOpExp>(ep)) {
        auto v1 = eval(vars, e->l);
        return v1->unop(e->op);
    }
    else if (auto e = dynamic_pointer_cast<MapDefExp>(ep)) {
        auto m = new ValueMap({});
        for (auto f : e->values) {
            m->getRef(f.first) = eval(vars, f.second);
        }
        return valp(m);
    }
    else if (auto e = dynamic_pointer_cast<ListDefExp>(ep)) {
        auto m = new ValueList({});
        for (int i=0;i<e->values.size();i++) {
            m->atRef(i) = eval(vars, e->values[i]);
        }
        return valp(m);
    } 
    else if (auto e = dynamic_pointer_cast<RangeDefExp>(ep)) {
        auto beg  = eval(vars, e->beg);
        auto end  = eval(vars, e->end);
        auto step = eval(vars, e->step);
        return valp(new ValueRange(beg->getInt(), end->getInt(), step->getInt()));
    }
    else if (auto e = dynamic_pointer_cast<FuncCallExp>(ep)) {
        // extract args
        vector<valp> args;
        for (auto a : e->a) {
            args.push_back(eval(vars, a));
        }
        // if no context call function globally
        if (!e->ctx) {
            if (dynamic_pointer_cast<ValueFunction>(vars->getRef(e->f))) return evalFunc(vars, e->f, args);
            else return evalFunc(variables, e->f, args);
        }
        // Get context
        valp vctx = eval(vars, e->ctx);
        // If context is a map call function
        if (dynamic_pointer_cast<ValueMap>(vctx)) {
            return evalFunc(vctx, e->f, args);
        // If not a map find method
        } else {
            return vctx->call(e->f, args);
        }
    }
    else if (auto e = dynamic_pointer_cast<StrExp>(ep)) {
        return valp(new ValueStr(e->v));
    }
    else if (auto e = dynamic_pointer_cast<TernaryExp>(ep)) {
        auto vcond = eval(vars, e->cond);
        if (vcond->isTrue()) return eval(vars, e->then);
        else return eval(vars, e->els);
    }
    else if (auto e = dynamic_pointer_cast<FuncDefExp>(ep)) {
        return valp(new ValueFunction(e->args, e->body));
    }
    else if (auto e = dynamic_pointer_cast<IndexExp>(ep)) {
        auto lv = eval(vars, e->l);
        auto iv = eval(vars, e->i);
        return lv->at(iv->getInt());
    }
    else if (auto e = dynamic_pointer_cast<MemberExp>(ep)) {
        auto lv = eval(vars, e->l);
        return lv->get(e->member);
    }
    else throw runtime_error("Unknown statement");
}

valp Script::eval(valp vars, expp ep) {
    try {
        valp v = eval1(vars, ep);
        // if values refer to outside references, return the value of these refs
        if (auto vv = dynamic_pointer_cast<ValueExternBase>(v)) {
            return vv->get();
        } else {
            return v;
        }
    } catch (runtime_error e) {
        throw InterpreterError(filename, source, ep->srcinfo, e.what());
    }
}

// Get references to values for assignment
valp& Script::evalRef(valp vars, expp lp) {
    try {
        if (auto l = dynamic_pointer_cast<IdExp>(lp)) {
            return vars->getRef(l->name);
        } else if (auto l = dynamic_pointer_cast<IndexExp>(lp)) {
            auto l0 = evalRef(vars, l->l);
            return l0->atRef(eval(vars, l->i)->getInt());
        } else if (auto l = dynamic_pointer_cast<MemberExp>(lp)) {
            auto l0 = evalRef(vars, l->l);
            return l0->getRef(l->member);
        }
        throw runtime_error("Can't get ref from this exp");
    } catch (runtime_error e) {
        throw InterpreterError(filename, source, lp->srcinfo, e.what());
    }
}

void Script::run() {
    exec(variables, code);
}

bool Script::isOver() {
    return false;
}