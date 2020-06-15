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
    virtual size_t length() { throw std::runtime_error("Not iterable");}
    virtual valp at(int id) { throw std::runtime_error("Not iterable");}
    virtual valp& atRef(int id) { throw std::runtime_error("Not iterable");}
    virtual valp get(std::string mem) { throw std::runtime_error("Can't get member from non-map");}
    virtual valp& getRef(std::string mem) { throw std::runtime_error("Can't get member from non-map");}
    virtual bool isTrue() { throw std::runtime_error("Can't evaluate to boolean");}
    virtual int getInt() { throw std::runtime_error("Not an int");}
    virtual valp call(std::string f, std::vector<valp> args) { throw std::runtime_error("Can't call function from this value");}
    virtual std::string getStr() { throw std::runtime_error("Not a string");}
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
    virtual int getInt() { return value; }
    virtual bool isTrue() { return value!=0; }
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
struct ValueMap : public Value {
    ValueMap(var vars) : vars(vars) {}
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    virtual valp binop(std::string op, valp r) { throw std::runtime_error("Unsupported");}
    virtual valp get(std::string mem) { 
        return vars[mem];
    }
    virtual valp &getRef(std::string mem) { 
        auto it = vars.find(mem);
        if (it == vars.end()) vars[mem] = valp(new ValueNone());
        return vars[mem];
    }
    virtual std::string print() {
        std::stringstream ss; 
        ss << "{";
        for (auto a : vars) {
            ss << a.first << ":" << a.second->print() << ";";
        }
        ss << "}";
        return ss.str();
    }
    var vars;
};

// Vector of values
struct ValueList : public Value {
    ValueList(std::vector<valp> values) : values(values) {}
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    virtual valp binop(std::string op, valp r) { throw std::runtime_error("Unsupported");}
    virtual size_t length() { return values.size(); }
    virtual valp at(int i) { return values.at(i); }
    virtual valp& atRef(int i) {
        if (i >= length()) values.resize(i+1);
        return values.at(i);
    }
    virtual valp call(std::string f, std::vector<valp> args) {
        if (f == "length" && args.size() == 0) return valp(new ValueInt(length()));
        throw std::runtime_error("Unknown method");
    }
    virtual std::string print() {
        std::stringstream ss; 
        ss << "[";
        for (auto a : values) {
            ss << a->print() << ",";
        }
        ss << "]";
        return ss.str();
    }
    std::vector<valp> values;
};

struct ValueRange : public Value {
    ValueRange(int beg, int end, int step) : beg(beg), end(end) {
        if (step == 0) throw std::runtime_error("Can't have a step of 0");
        this->step = step;
    }
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    virtual valp binop(std::string op, valp r) { throw std::runtime_error("Unsupported");}
    virtual size_t length() { return ((end-beg)/step)+1; }
    virtual valp at(int i) { return valp(new ValueInt(beg + step*i));}
    virtual valp& atRef(int id) { throw std::runtime_error("Can't access range as left-value");}
    virtual std::string print() {
        std::stringstream ss; 
        ss << "[" << beg << ".." << end;
        if (step != 1) ss << ".." << step;
        ss << "]";
        return ss.str();
    }
    int beg, end, step;
};

// String
struct ValueStr : public Value {
    ValueStr(std::string v) : value(v) {}
    virtual valp unop(std::string op) { throw std::runtime_error("Unsupported");}
    virtual valp binop(std::string op, valp r);
    virtual std::string getStr() { return value; }
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

struct SourceInfo {
    size_t line = -1;
    size_t column;
    size_t start_index;
    size_t end_index;
};

class InterpreterError : public std::exception {
public:
    InterpreterError(std::string filename, std::string source, SourceInfo srcinfo, const std::string &str);
    virtual const char* what() const noexcept;
private:
    std::string str;
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

    // Links reference to script variable
    template <typename T>
    void link(std::string name, T& ref) {
        variables->getRef(name) = valp(new ValueExtern<T>(ref));
    }
    // Links native function to script variable
    template <typename T>
    void linkFunction(std::string name, std::function<T> f) {
        // Create wrapper function that takes list of values and returns value
        variables->getRef(name) = valp(new ValueNativeFunc([&](auto a) {
            return call(f, a);
        }));
    }

private:
    void load(std::string path);
    // Executes statement s with context vars
    void exec(valp vars, statp s);
    // Evaluates expresison e with context vars
    valp eval(valp vars, expp e);
    valp eval1(valp vars, expp e);

    valp& evalRef(valp vars, expp lp);

    valp evalFunc(valp ctx, std::string f, std::vector<valp> args);

    // Current return value; null means not returning
    valp ret = nullptr;
    // Script variables
    valp variables = valp(new ValueMap({}));
    // AST to execute
    statp code;
    std::string source;
    std::string filename;
};