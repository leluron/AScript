#include <antlr4-runtime/antlr4-runtime.h>
#include "parser/ASParser.h"
#include "parser/ASLexer.h"
#include "parser/ASBaseVisitor.h"

#include <ascript/script.h>

#define BINOP return expp(new BinOpExp(ctx->op->getText(), visit(ctx->exp(0)), visit(ctx->exp(1))))
#define UNOP return expp(new UnOpExp(ctx->op->getText(), visit(ctx->exp())))

using namespace std;

// Generate AST from antlr4 contexts
class ASTGen : ASBaseVisitor {
public:
    virtual antlrcpp::Any visitFile(ASParser::FileContext *ctx) override {
        vector<statp> s;
        for (auto c : ctx->stat()) {
            s.push_back(visit(c));
        }
        return statp(new BlockStat(s));
    }

    virtual antlrcpp::Any visitAssignstat(ASParser::AssignstatContext *ctx) override {
        return statp(new AssignStat(
            visit(ctx->exp(0)).as<expp>(),
            visit(ctx->exp(1)).as<expp>()
        ));
    }

    // Return stat, or empty block if ctx is null
    statp stat_option(ASParser::StatContext *s) {
        if (!s) return statp(new BlockStat({}));
        else return visit(s);
    }

    // convert multiple if/(else if)/else blocks into nested if/else
    statp visitIfAux(ASParser::CondstatContext *ctx, int index) {
        expp e = visit(ctx->exp(index));
        statp s = visit(ctx->stat(index));
        if (index == ctx->exp().size()) {
            statp els = stat_option(ctx->els);
            return statp(new IfStat(e,s,els));
        } else {
            return statp(new IfStat(e,s,visitIfAux(ctx, index+1)));
        }
    }

    virtual antlrcpp::Any visitCondstat(ASParser::CondstatContext *ctx) override {
        return visitIfAux(ctx, 0);
    }

    virtual antlrcpp::Any visitBlockstat(ASParser::BlockstatContext *ctx) override {
        vector<statp> s;
        for (auto c : ctx->stat()) {
            s.push_back(visit(c));
        }
        return statp(new BlockStat(s));
    }

    virtual antlrcpp::Any visitWhilestat(ASParser::WhilestatContext *ctx) override {
        return statp(new WhileStat(
            visit(ctx->exp()).as<expp>(),
            statp(visit(ctx->stat()).as<statp>())
        ));
    }

    virtual antlrcpp::Any visitReturnstat(ASParser::ReturnstatContext *ctx) override {
        expp ret = nullptr;
        if (ctx->exp()) ret = visit(ctx->exp());
        return statp(new ReturnStat(ret));
    }

    virtual antlrcpp::Any visitIdexp(ASParser::IdexpContext *ctx) override {
        return expp(new IdExp(ctx->ID()->getText()));
    }

    virtual antlrcpp::Any visitIntexp(ASParser::IntexpContext *ctx) override {
        stringstream ss;
        ss << ctx->INT()->getText();
        int v;
        ss >> v;
        return expp(new IntExp(v));
    }
    virtual antlrcpp::Any visitFloatexp(ASParser::FloatexpContext *ctx) override {
        stringstream ss;
        ss << ctx->FLOAT()->getText();
        float v;
        ss >> v;
        return expp(new FloatExp(v));
    }
    virtual antlrcpp::Any visitFunccallstat(ASParser::FunccallstatContext *ctx) override {
        return statp(new FuncCallStat(nullptr, ctx->ID()->getText(), visit(ctx->explist())));
    }
    virtual antlrcpp::Any visitMembercallstat(ASParser::MembercallstatContext *ctx) override {
        return statp(new FuncCallStat(visit(ctx->exp()), ctx->ID()->getText(), visit(ctx->explist())));
    }

    virtual antlrcpp::Any visitMapdef(ASParser::MapdefContext *ctx) override {
        map<string, expp> v;
        for (int i=0;i<ctx->ID().size();i++) {
            v[ctx->ID(i)->getText()] = visit(ctx->exp(i));
        }
        return expp(new MapDefExp(v));
    }

    virtual antlrcpp::Any visitFunctiondef(ASParser::FunctiondefContext *ctx) override {
        return expp(new FuncDefExp(visit(ctx->idlist()), visit(ctx->stat())));
    }

    virtual antlrcpp::Any visitFunccallexp(ASParser::FunccallexpContext *ctx) override {
        return expp(new FuncCallExp(nullptr, ctx->ID()->getText(), visit(ctx->explist())));
    }

    virtual antlrcpp::Any visitMembercallexp(ASParser::MembercallexpContext *ctx) override {
        return expp(new FuncCallExp(visit(ctx->exp()), ctx->ID()->getText(), visit(ctx->explist())));
    }

    virtual antlrcpp::Any visitUnaryexp(ASParser::UnaryexpContext *ctx) override {
        UNOP;
    }
    virtual antlrcpp::Any visitAndexp(ASParser::AndexpContext *ctx) override {
        BINOP;
    }
    virtual antlrcpp::Any visitOrexp(ASParser::OrexpContext *ctx) override {
        BINOP;
    }
    virtual antlrcpp::Any visitAdditiveexp(ASParser::AdditiveexpContext *ctx) override {
        BINOP;
    }
    virtual antlrcpp::Any visitRelationexp(ASParser::RelationexpContext *ctx) override {
        BINOP;
    }
    virtual antlrcpp::Any visitMultiplicativeexp(ASParser::MultiplicativeexpContext *ctx) override {
        BINOP;
    }
    virtual antlrcpp::Any visitComparisonexp(ASParser::ComparisonexpContext *ctx) override {
        BINOP;
    }
    virtual antlrcpp::Any visitTrueexp(ASParser::TrueexpContext *ctx) override {
        return expp(new IntExp(1));
    }
    virtual antlrcpp::Any visitFalseexp(ASParser::FalseexpContext *ctx) override {
        return expp(new IntExp(0));
    }
    virtual antlrcpp::Any visitStringexp(ASParser::StringexpContext *ctx) override {
        auto str = ctx->STRING()->getText();
        return expp(new StrExp(str.substr(1, str.length()-1)));
    }
    virtual antlrcpp::Any visitParenexp(ASParser::ParenexpContext *ctx) override {
        return visit(ctx->exp());
    }
    virtual antlrcpp::Any visitMemberexp(ASParser::MemberexpContext *ctx) override {
        return expp(new IndexExp(visit(ctx->exp()), expp(new StrExp(ctx->ID()->getText()))));
    }
    virtual antlrcpp::Any visitExplist(ASParser::ExplistContext *ctx) override {
        expl l;
        for (auto e : ctx->exp()) {
            l.push_back(visit(e));
        }
        return l;
    }
    virtual antlrcpp::Any visitIdlist(ASParser::IdlistContext *ctx) override {
        std::vector<std::string> l;
        for (auto i : ctx->ID()) {
            l.push_back(i->getText());
        }
        return l;
    }

    virtual antlrcpp::Any visitTernaryexp(ASParser::TernaryexpContext *ctx) override {
        return expp(new TernaryExp(visit(ctx->exp(0)), visit(ctx->exp(1)), visit(ctx->exp(2))));
    }

};

statp toAST(ASParser::FileContext *file) {
    return ASTGen().visitFile(file);
}