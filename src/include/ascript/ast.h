#pragma once

#include <vector>
#include <string>

struct SourceInfo {
    size_t line = -1;
    size_t column;
    size_t start_index;
    size_t end_index;
};

struct Stat {
    virtual ~Stat() {}
    SourceInfo srcinfo;
};

struct Exp {
    virtual ~Exp() {}
    SourceInfo srcinfo;
};

// left = right
struct AssignStat : public Stat {
    AssignStat(expp l, expp r) : left(l), right(r) {}
    expp left;
    expp right;
};

struct CompAssignStat : public Stat {
    CompAssignStat(expp l, expp r, std::string op) : left(l), right(r), op(op) {}
    expp left, right;
    std::string op;
};

// if cond then else els
struct IfStat : public Stat {
    IfStat(expp cond, statp then, statp els) : cond(cond), then(then), els(els) {}
    expp cond;
    statp then, els;
};

// { stats... }
struct BlockStat : public Stat {
    BlockStat(std::vector<statp> stats) : stats(stats) {}
    std::vector<statp> stats;
};

// while cond stat
struct WhileStat : public Stat {
    WhileStat(expp cond, statp stat) : cond(cond), stat(stat) {}
    expp cond;
    statp stat;
};

struct ForStat : public Stat {
    ForStat(std::string id, expp list, statp stat) : id(id), list(list), stat(stat) {}
    std::string id;
    expp list;
    statp stat;
};

// ctx.f(a...)
// or
// f(a...)
struct FuncCallStat : public Stat {
    FuncCallStat(expp ctx, std::string f, expl a) : ctx(ctx), f(f), a(a) {}
    expp ctx;
    std::string f;
    expl a;
};

// return e
// or
// return
struct ReturnStat : public Stat {
    ReturnStat(expp e) : e(e) {}
    expp e;
};

// Int literal
struct IntExp : public Exp {
    IntExp(int v) : value(v) {}
    int value;
};

// Float literal
struct FloatExp : public Exp {
    FloatExp(int v) : value(v) {}
    float value;
};

// Variable name
struct IdExp : public Exp {
    IdExp(std::string v) : name(v) {}
    std::string name;
};

// Binary operation
struct BinOpExp : public Exp {
    BinOpExp(std::string op, expp l, expp r) : op(op), l(l), r(r) {}
    std::string op;
    expp l,r;
};

// Unary operation
struct UnOpExp : public Exp {
    UnOpExp(std::string op, expp l) : op(op), l(l) {}
    std::string op;
    expp l;
};

// Map constructor
struct MapDefExp : public Exp {
    MapDefExp(std::map<std::string, expp> values) : values(values) {}
    std::map<std::string, expp> values;
};

// List constructor
struct ListDefExp : public Exp {
    ListDefExp(expl values) : values(values) {}
    expl values;
};

// Range constructor
struct RangeDefExp : public Exp {
    RangeDefExp(expp beg, expp end, expp step) : beg(beg), end(end), step(step) {}
    expp beg, end, step;
};

// ctx.f(a...)
// or
// f(a...)
struct FuncCallExp : public Exp {
    FuncCallExp(expp ctx, std::string f , expl a) : ctx(ctx), f(f), a(a) {}
    expp ctx;
    std::string f;
    expl a;
};

// function(args...) body
struct FuncDefExp : public Exp {
    FuncDefExp(std::vector<std::string> args, statp body) : args(args), body(body) {}
    std::vector<std::string> args;
    statp body;
};

// String literal
struct StrExp : public Exp {
    StrExp(std::string v) : v(v) {}
    std::string v;
};

// l[i]
struct IndexExp : public Exp {
    IndexExp(expp l, expp i) : l(l), i(i) {}
    expp l,i;
};

// l.member
struct MemberExp : public Exp {
    MemberExp(expp l, std::string member) : l(l), member(member) {}
    expp l;
    std::string member;
};

// then if cond else els
struct TernaryExp : public Exp {
    TernaryExp(expp cond, expp then, expp els) : cond(cond), then(then), els(els) {}
    expp cond, then, els;
};