#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <sstream>

// Runtime value
class Value;
// Any value
using valp = std::shared_ptr<Value>;

// AST
struct Stat;
struct Exp;

// Any statement
using statp = std::shared_ptr<Stat>;
// Any expression
using expp = std::shared_ptr<Exp>;
// List of expressions
using expl = std::vector<expp>;
// Variables (names associated to values)
using var = std::map<std::string, valp>;

struct Value {
    virtual ~Value() {};
    // Unary operator
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    // Binary operator
    virtual valp binop(std::string op, valp r) { throw std::runtime_error("Unsupported");}
    virtual std::string print() = 0;
};

struct ValueNone : public Value {
    virtual std::string print() {
        return "None";
    }
};

struct ValueInt : public Value {
    ValueInt(int v) : value(v) {}
    virtual valp unop(std::string op);
    virtual valp binop(std::string op, valp r);
    virtual std::string print() {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
    int value;
};

struct ValueFloat : public Value {
    ValueFloat(float v) : value(v) {}
    virtual valp unop(std::string op);
    virtual valp binop(std::string op, valp r);
    virtual std::string print() {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
    float value;
};

// Names associated to values
struct ValueMap : public Value, public var {
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    virtual valp binop(std::string op, valp r) { throw std::runtime_error("Unsupported");}
    virtual std::string print() {
        std::stringstream ss; 
        ss << "{\n";
        for (auto a : *this) {
            ss << a.first << ":" << a.second->print() << "\n";
        }
        ss << "}";
        return ss.str();
    }
};

// String
struct ValueStr : public Value {
    ValueStr(std::string v) : value(v) {}
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    virtual valp binop(std::string op, valp r);
    virtual std::string print() {
        std::stringstream ss;
        ss << "\"" << value << "\"";
        return ss.str();
    }
    std::string value;
};

// Calls script function
struct ValueFunction : public Value {
    /* args = names of arguments
       body = function body */
    ValueFunction(std::vector<std::string> args, statp body) : args(args), body(body) {}
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    virtual valp binop(std::string op, valp r) { throw std::runtime_error("Unsupported");}
    virtual std::string print() {
        std::stringstream ss;
        ss << "function(" << args.size() << ")";
        return ss.str();
    }
    std::vector<std::string> args;
    statp body;
};

// Calls native function
struct ValueNativeFunc : public Value  {
    /* f = native function wrapper, takes a list of values as arguments, returns any value */
    ValueNativeFunc(std::function<valp(std::vector<valp>)> f) : f(f) {}
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    virtual valp binop(std::string op, valp r) { throw std::runtime_error("Unsupported");}
    std::function<valp(std::vector<valp>)> f;
    virtual std::string print() {
        return "nativefunction";
    }
};

// Reference to native variable
template <typename T>
struct ValueExtern : public Value {
    ValueExtern(T& ref) : ref(ref) {}
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    virtual valp binop(std::string op, valp r) { throw std::runtime_error("Unsupported");}
    virtual std::string print() {
        std::string s = "externvalue<";
        s += typeid(T).name();
        s += ">";
        return s;
    }
    T& ref;
};

struct Stat {
    virtual ~Stat() {}
};

struct Exp {
    virtual ~Exp() {}
};

// left = right
struct AssignStat : public Stat {
    AssignStat(expp l, expp r) : left(l), right(r) {}
    expp left;
    expp right;
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
// or
// l.i
struct IndexExp : public Exp {
    IndexExp(expp l, expp i) : l(l), i(i) {}
    expp l,i;
};

// then if cond else els
struct TernaryExp : public Exp {
    TernaryExp(expp cond, expp then, expp els) : cond(cond), then(then), els(els) {}
    expp cond, then, els;
};

// Convert return value into script value
template<typename T> 
valp convertRet(T a);

// Unwrap script value
template <typename T>
T convert(valp a);

// Call native function with converted arguments
template <typename Ret, typename ...Args, size_t ...I>
Ret call0(std::function<Ret(Args...)> f, std::vector<valp>& a, std::index_sequence<I...>) {
    return f(convert<Args>(a[I])...);
}

// Call f by converting arguments a to native types, get the result and wrap it
template<typename Ret, typename ...Args>
valp call(std::function<Ret(Args...)> f, std::vector<valp>& a) {
    // Check number of arguments
    if (a.size() != sizeof...(Args)) throw std::runtime_error("Unmatched argument number");
    return convertRet(call0(f, a, std::make_index_sequence<sizeof...(Args)>{}));
}

class Script {
public:
    // Loads script from path
    Script(std::string path);
    // Launches script
    void run();
    // Returns whether script has finished
    bool isOver();
    // reference to script variables
    ValueMap& getVars() { return *std::dynamic_pointer_cast<ValueMap>(variables); }

    // Links reference to script variable
    template <typename T>
    void link(std::string name, T& ref) {
        getVars()[name] = valp(new ValueExtern<T>(ref));
    }
    // Links native function to script variable
    template <typename T>
    void linkFunction(std::string name, std::function<T> f) {
        // Create wrapper function that takes list of values and returns value
        getVars()[name] = valp(new ValueNativeFunc([&](auto a) {
            return call(f, a);
        }));
    }

private:
    // Executes statement s with context vars
    void exec(valp vars, statp s);
    // Evaluates expresison e with context vars
    valp eval(valp vars, expp e);
    // Get lvalue of l with context vars
    // Create variable if not found; if create flag set
    valp &getRef(valp vars, expp l, bool create);

    // Current return value; null means not returning
    valp ret = nullptr;
    // Script variables
    valp variables = valp(new ValueMap());
    // AST to execute
    statp code;

};