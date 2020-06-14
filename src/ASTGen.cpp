#include <antlr4-runtime/antlr4-runtime.h>
#include "parser/ASParser.h"
#include "parser/ASLexer.h"
#include "parser/ASBaseVisitor.h"

#include <ascript/script.h>

#define BINOP return exp(new BinOpExp(ctx->op->getText(), visit(ctx->exp(0)), visit(ctx->exp(1))), ctx)
#define UNOP return exp(new UnOpExp(ctx->op->getText(), visit(ctx->exp())), ctx)

using namespace std;

// Generate AST from antlr4 contexts
class ASTGen : ASBaseVisitor {
public:
    SourceInfo get(antlr4::ParserRuleContext *ctx) {
        return {
            ctx->getStart()->getLine(),
            ctx->getStart()->getCharPositionInLine(),
            ctx->getStart()->getStartIndex(),
            ctx->getStop()->getStopIndex()
        };
    }

    expp exp(Exp *e, antlr4::ParserRuleContext *ctx) {
        e->srcinfo = get(ctx);
        return expp(e);
    }

    statp stat(Stat *s, antlr4::ParserRuleContext *ctx) {
        s->srcinfo = get(ctx);
        return statp(s);
    }

    virtual antlrcpp::Any visitFile(ASParser::FileContext *ctx) override {
        vector<statp> s;
        for (auto c : ctx->stat()) {
            s.push_back(visit(c));
        }
        return stat(new BlockStat(s), ctx);
    }

    virtual antlrcpp::Any visitAssignstat(ASParser::AssignstatContext *ctx) override {
        return stat(new AssignStat(
            visit(ctx->exp(0)).as<expp>(),
            visit(ctx->exp(1)).as<expp>()
        ), ctx);
    }

    virtual antlrcpp::Any visitCompassignstat(ASParser::CompassignstatContext *ctx) override {
        return stat(new CompAssignStat(
            visit(ctx->exp(0)),
            visit(ctx->exp(1)),
            ctx->op->getText()
        ), ctx);
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
        if (index == ctx->exp().size()-1) {
            statp els = stat_option(ctx->els);
            return stat(new IfStat(e,s,els), ctx);
        } else {
            return stat(new IfStat(e,s,visitIfAux(ctx, index+1)), ctx);
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
        return stat(new BlockStat(s), ctx);
    }

    virtual antlrcpp::Any visitWhilestat(ASParser::WhilestatContext *ctx) override {
        return stat(new WhileStat(
            visit(ctx->exp()).as<expp>(),
            visit(ctx->stat()).as<statp>()
        ), ctx);
    }

    virtual antlrcpp::Any visitForstat(ASParser::ForstatContext *ctx) override {
        return stat(new ForStat(
            ctx->ID()->getText(),
            visit(ctx->exp()),
            visit(ctx->stat())
        ), ctx);
    }

    virtual antlrcpp::Any visitReturnstat(ASParser::ReturnstatContext *ctx) override {
        expp ret = nullptr;
        if (ctx->exp()) ret = visit(ctx->exp());
        return stat(new ReturnStat(ret), ctx);
    }

    virtual antlrcpp::Any visitIdexp(ASParser::IdexpContext *ctx) override {
        return exp(new IdExp(ctx->ID()->getText()), ctx);
    }

    virtual antlrcpp::Any visitIntexp(ASParser::IntexpContext *ctx) override {
        stringstream ss;
        ss << ctx->INT()->getText();
        int v;
        ss >> v;
        return exp(new IntExp(v), ctx);
    }
    virtual antlrcpp::Any visitFloatexp(ASParser::FloatexpContext *ctx) override {
        stringstream ss;
        ss << ctx->FLOAT()->getText();
        float v;
        ss >> v;
        return exp(new FloatExp(v), ctx);
    }
    virtual antlrcpp::Any visitFunccallstat(ASParser::FunccallstatContext *ctx) override {
        return stat(new FuncCallStat(nullptr, ctx->ID()->getText(), visit(ctx->explist())), ctx);
    }
    virtual antlrcpp::Any visitMembercallstat(ASParser::MembercallstatContext *ctx) override {
        return stat(new FuncCallStat(visit(ctx->exp()), ctx->ID()->getText(), visit(ctx->explist())), ctx);
    }

    virtual antlrcpp::Any visitMapdef(ASParser::MapdefContext *ctx) override {
        map<string, expp> v;
        for (int i=0;i<ctx->ID().size();i++) {
            v[ctx->ID(i)->getText()] = visit(ctx->exp(i));
        }
        return exp(new MapDefExp(v), ctx);
    }

    virtual antlrcpp::Any visitListdef(ASParser::ListdefContext *ctx) override {
        expl e = {};
        if (ctx->explist()) e = visit(ctx->explist()).as<expl>();
        return exp(new ListDefExp(e), ctx);
    }

    virtual antlrcpp::Any visitIndexexp(ASParser::IndexexpContext *ctx) override {
        return exp(new IndexExp(visit(ctx->exp(0)), visit(ctx->exp(1))), ctx);
    }

    virtual antlrcpp::Any visitFunctiondef(ASParser::FunctiondefContext *ctx) override {
        vector<string> idlist;
        if (ctx->idlist()) idlist = visit(ctx->idlist()).as<vector<string>>();
        return exp(new FuncDefExp(idlist, visit(ctx->stat())), ctx);
    }

    virtual antlrcpp::Any visitFunccallexp(ASParser::FunccallexpContext *ctx) override {
        expl args = (ctx->explist())?visit(ctx->explist()).as<expl>():expl();
        return exp(new FuncCallExp(nullptr, ctx->ID()->getText(), args), ctx);
    }

    virtual antlrcpp::Any visitMembercallexp(ASParser::MembercallexpContext *ctx) override {
        expl args = (ctx->explist())?visit(ctx->explist()).as<expl>():expl();
        return exp(new FuncCallExp(visit(ctx->exp()), ctx->ID()->getText(), args), ctx);
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
        return exp(new IntExp(1), ctx);
    }
    virtual antlrcpp::Any visitFalseexp(ASParser::FalseexpContext *ctx) override {
        return exp(new IntExp(0), ctx);
    }
    virtual antlrcpp::Any visitStringexp(ASParser::StringexpContext *ctx) override {
        auto str = ctx->STRING()->getText();
        return exp(new StrExp(str.substr(1, str.length()-2)), ctx);
    }
    virtual antlrcpp::Any visitParenexp(ASParser::ParenexpContext *ctx) override {
        return visit(ctx->exp());
    }
    virtual antlrcpp::Any visitMemberexp(ASParser::MemberexpContext *ctx) override {
        return exp(new IndexExp(visit(ctx->exp()), expp(new StrExp(ctx->ID()->getText()))), ctx);
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
        return exp(new TernaryExp(visit(ctx->exp(1)), visit(ctx->exp(0)), visit(ctx->exp(2))), ctx);
    }

};

statp toAST(ASParser::FileContext *file) {
    return ASTGen().visitFile(file);
}