#include <ascript/script.h>
#include <istream>
#include <vector>

#include <antlr4-runtime/antlr4-runtime.h>
#include "parser/ASParser.h"
#include "parser/ASLexer.h"

using namespace std;

// Generic unop
template <typename T> T unop0(T l, string op) {
    if (op == "-") return -l;
    if (op == "not") return (l==0);
    throw runtime_error("unknown op");
}

valp ValueInt::unop(string op) {
    return valp(new ValueInt(unop0(value, op)));
}

valp ValueFloat::unop(string op) {
    return valp(new ValueFloat(unop0(value, op)));
}

// Generic binop
template <typename T> T binop0(T l, T r, string op) {
    if (op == "+") return l+r;
    if (op == "-") return l-r;
    if (op == "*") return l*r;
    if (op == "/") return l/r;
    if (op == "%") return (int)l%(int)r;
    if (op == "<=") return l<=r;
    if (op == ">=") return l>=r;
    if (op == "<") return l<r;
    if (op == ">") return l>r;
    if (op == "and") return l&&r;
    if (op == "or" ) return l||r;
    throw runtime_error("unknown op");
}

valp ValueInt::binop(string op, valp rp) {
    if (auto r = dynamic_pointer_cast<ValueInt>(rp))
        return valp(new ValueInt(binop0(value, r->value, op)));
    if (auto r = dynamic_pointer_cast<ValueFloat>(rp))
        return valp(new ValueFloat(binop0((float)value, r->value, op)));
    throw runtime_error("Unsupported operation");
}

valp ValueFloat::binop(string op, valp rp) {
    if (auto r = dynamic_pointer_cast<ValueInt>(rp))
        return valp(new ValueFloat(binop0(value, (float)r->value, op)));
    if (auto r = dynamic_pointer_cast<ValueFloat>(rp))
        return valp(new ValueFloat(binop0(value, r->value, op)));
    throw runtime_error("Unsupported operation");
}

// String concat
valp ValueStr::binop(string op, valp rp) {
    if (auto r = dynamic_pointer_cast<ValueStr>(rp))
        return valp(new ValueStr(value + r->value));
    throw runtime_error("Unsupported operation");
}

// Extract AST from antlr context
statp toAST(ASParser::FileContext *file); 

// Load AST from file
statp load(string path) {
    ifstream stream(path);
    antlr4::ANTLRInputStream input(stream);
    ASLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ASParser parser(&tokens);
    ASParser::FileContext* tree = parser.file();

    return toAST(tree);
}

Script::Script(string path) {
    code = load(path);
}

void Script::exec(valp vars,statp sp) {
    if (auto s = dynamic_pointer_cast<AssignStat>(sp)) {
        // eval right side
        auto r = eval(vars, s->right);
        // get reference to left side
        auto& v = getRef(vars, s->left, true);
        // Specialization for extern values
        if (auto vv = dynamic_pointer_cast<ValueExtern<int>>(v)) {
            if (auto rv = dynamic_pointer_cast<ValueInt>(r))
                vv->ref = rv->value;
            else if (auto rv = dynamic_pointer_cast<ValueFloat>(r))
                vv->ref = (int)rv->value;
            else throw runtime_error("Uncompatible types");
        } else if (auto vv = dynamic_pointer_cast<ValueExtern<float>>(v)) {
            if (auto rv = dynamic_pointer_cast<ValueInt>(r))
                vv->ref = (float)rv->value;
            else if (auto rv = dynamic_pointer_cast<ValueFloat>(r))
                vv->ref = rv->value;
            else throw runtime_error("Uncompatible types");
        } else if (auto vv = dynamic_pointer_cast<ValueExtern<string>>(v)) {
            if (auto rv = dynamic_pointer_cast<ValueStr>(r))
                vv->ref = rv->value;
            else throw runtime_error("Uncompatible types");
        } else {
            // if no extern just point to right side value
            v = r;
        }
    }
    else if (auto s = dynamic_pointer_cast<FuncCallStat>(sp)) {
        eval(vars, expp(new FuncCallExp(s->ctx, s->f, s->a)));
    }
    else if (auto s = dynamic_pointer_cast<IfStat>(sp)) {
        auto v = eval(vars, s->cond);
        if (auto vi = dynamic_pointer_cast<ValueInt>(v)) {
            if (vi->value) exec(vars, s->then);
            else exec(vars, s->els);
        } else throw runtime_error("Can't evaluate condition with non-int");
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
            auto v = eval(vars, s->cond);
            if (auto vi = dynamic_pointer_cast<ValueInt>(v)) {
                if (vi->value) {
                    exec(vars, s->stat);
                    // stop loop if return stat executed
                    if (ret) return;
                }
                else break;
            } else throw runtime_error("Can't evaluate condition with non-int");
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
}

valp Script::eval(valp vars0, expp ep) {
    valp v;

    ValueMap& vars = *dynamic_pointer_cast<ValueMap>(vars0);

    if (auto e = dynamic_pointer_cast<IntExp>(ep)) {
        v = valp(new ValueInt(e->value));
    }
    else if (auto e = dynamic_pointer_cast<FloatExp>(ep)) {
        v = valp(new ValueFloat(e->value));
    }
    else if (auto e = dynamic_pointer_cast<IdExp>(ep)) {
        auto it = vars.find(e->name);
        if (it == vars.end()) throw runtime_error("Unknown variable");
        v = it->second;
    }
    else if (auto e = dynamic_pointer_cast<BinOpExp>(ep)) {
        auto v1 = eval(vars0, e->l);
        auto v2 = eval(vars0, e->r);
        v = v1->binop(e->op, v2);
    }
    else if (auto e = dynamic_pointer_cast<UnOpExp>(ep)) {
        auto v1 = eval(vars0, e->l);
        v = v1->unop(e->op);
    }
    else if (auto e = dynamic_pointer_cast<MapDefExp>(ep)) {
        auto m = new ValueMap({});
        for (auto f : e->values) {
            m->insert(make_pair(f.first, eval(vars0, f.second)));
        }
        v = valp(m);
    }
    else if (auto e = dynamic_pointer_cast<FuncCallExp>(ep)) {
        // Get context for `this`
        valp vctx = (e->ctx)?eval(vars0, e->ctx):vars0;
        // Check if `this` is a map
        ValueMap vvctx;
        if (auto vctx1 = dynamic_pointer_cast<ValueMap>(vctx)) {
            vvctx = *vctx1;
        } else throw runtime_error("Can't call member function on non-map");

        // extract function or method
        auto f0 = vvctx.at(e->f);
        if (auto f = dynamic_pointer_cast<ValueFunction>(f0)) {
            // In case of script function
            // Extract arguments
            vector<valp> args;
            for (auto a : e->a) {
                args.push_back(eval(vars0, a));
            }
            // place arguments in a map associated with argument names
            ValueMap *env = new ValueMap();
            if (f->args.size() != args.size()) throw runtime_error("Unmatching arguments");
            for (int i=0;i<f->args.size();i++) {
                (*env)[f->args[i]] = args[i];
            }
            // link `this`
            (*env)["this"] = vctx;
            // run function
            exec(valp(env), f->body);
            // extract return value
            v = ret;
            // as we come back to the underlying code reset return indicator
            ret = nullptr;
        } else if (auto f = dynamic_pointer_cast<ValueNativeFunc>(f0)) {
            // in case of native function
            // extract arguments
            vector<valp> args;
            for (auto a : e->a) {
                args.push_back(eval(vars0, a));
            }
            // run function and get return value
            v = f->f(args);
        } else throw runtime_error("Can't call non-function");
    }
    else if (auto e = dynamic_pointer_cast<StrExp>(ep)) {
        v = valp(new ValueStr(e->v));
    }
    else if (auto e = dynamic_pointer_cast<TernaryExp>(ep)) {
        auto v = eval(vars0, e->cond);
        if (auto vi = dynamic_pointer_cast<ValueInt>(v)) {
            if (vi->value) v = eval(vars0, e->then);
            else v = eval(vars0, e->els);
        } else throw runtime_error("Can't evaluate condition with non-int");
    }
    else if (auto e = dynamic_pointer_cast<FuncDefExp>(ep)) {
        v = valp(new ValueFunction(e->args, e->body));
    }
    else if (auto e = dynamic_pointer_cast<IndexExp>(ep)) {
        auto lv = eval(vars0, e->l);
        auto iv = eval(vars0, e->i);
        if (auto l = dynamic_pointer_cast<ValueMap>(lv)) {
            if (auto i = dynamic_pointer_cast<ValueStr>(iv)) {
                v = l->at(i->value);
            }
            else throw runtime_error("Unmatching types");
        }
        else throw runtime_error("Unmatching types");
    }
    else throw runtime_error("Unknown statement");

    // if values refer to outside references, return the value of these refs
    if (auto vv = dynamic_pointer_cast<ValueExtern<int>>(v)) {
        return valp(new ValueInt(vv->ref));
    } else if (auto vv = dynamic_pointer_cast<ValueExtern<float>>(v)) {
        return valp(new ValueFloat(vv->ref));
    } else if (auto vv = dynamic_pointer_cast<ValueExtern<string>>(v)) {
        return valp(new ValueStr(vv->ref));
    } else {
        return v;
    }
}

// TODO change grammar to reflect lvalues most, and allow creation of members of list elements
valp& Script::getRef(valp vars0, expp lp, bool create) {
    ValueMap& vars = *dynamic_pointer_cast<ValueMap>(vars0);

    if (auto l = dynamic_pointer_cast<IdExp>(lp)) {
        auto it = vars.find(l->name);
        if (it == vars.end()) {
            if (!create) throw runtime_error("No var or member");
            auto it2 = vars.insert(pair<string, valp>(l->name, nullptr));
            return it2.first->second;
        } else {
            return it->second;
        }
    } else if (auto l = dynamic_pointer_cast<IndexExp>(lp)) {
        auto l0 = getRef(vars0, l->l, false);
        if (auto l1 = dynamic_pointer_cast<ValueMap>(l0)) {
            auto ei = eval(vars0, l->i);
            if (auto e = dynamic_pointer_cast<ValueStr>(ei)) {
                auto it = l1->find(e->value);
                if (it == vars.end()) {
                    if (!create) throw runtime_error("No var or member");
                    auto it2 = l1->insert(pair<string, valp>(e->value, nullptr));
                    return it2.first->second;
                } else {
                    return it->second;
                }
            }
            throw runtime_error("Invalid exp");
        }
        throw runtime_error("Can't access member");
    }
    throw runtime_error("Can't get ref from this exp");
}

void Script::run() {
    exec(variables, code);
}

bool Script::isOver() {
    return false;
}

// Converters between script values and native values
template <typename T> valp convertRet_int(T a) {
    return valp(new ValueInt(a));
}

template <typename T> valp convertRet_float(T a) {
    return valp(new ValueFloat(a));
}

template <>
valp convertRet(int a) {
    return convertRet_int(a);
}

template <>
valp convertRet(float a) {
    return convertRet_float(a);
}

template <>
valp convertRet(string a) {
    return valp(new ValueStr(a));
}

template <typename T> T convert_int(valp a) {
    if (auto a0 = dynamic_pointer_cast<ValueInt>(a)) return a0->value;
    throw runtime_error("Unmatched argument types");
}

template <typename T> T convert_float(valp a) {
    if (auto a0 = dynamic_pointer_cast<ValueFloat>(a)) return a0->value;
    throw runtime_error("Unmatched argument types");
}

template<>
int convert(valp a) {
    return convert_int<int>(a);
}

template<>
float convert(valp a) {
    return convert_float<float>(a);
}

template<>
string convert(valp a) {
    if (auto a0 = dynamic_pointer_cast<ValueStr>(a)) return a0->value;
    throw runtime_error("Unmatched argument types");
}