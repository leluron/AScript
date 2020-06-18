#pragma once

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <stdexcept>

// Runtime value
struct Value;
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
    virtual valp unop(std::string op) ;
    // Binary operator
    virtual valp binop(std::string op, valp r) ;
    virtual size_t length() ;
    virtual valp at(int id) ;
    virtual valp& atRef(int id) ;
    virtual valp get(std::string mem) ;
    virtual valp& getRef(std::string mem) ;
    virtual bool isTrue() ;
    virtual int getInt() ;
    virtual valp call(std::string f, std::vector<valp> args) ;
    virtual std::string getStr() ;
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
    virtual std::string print();
    int value;
};

struct ValueFloat : public Value {
    ValueFloat(float v) : value(v) {}
    virtual valp unop(std::string op);
    virtual valp binop(std::string op, valp r);
    virtual std::string print();
    float value;
};

// Names associated to values
struct ValueMap : public Value {
    ValueMap(var vars) : vars(vars) {}
    virtual valp get(std::string mem);
    virtual valp &getRef(std::string mem);
    virtual std::string print();
    var vars;
};

// Vector of values
struct ValueList : public Value {
    ValueList(std::vector<valp> values) : values(values) {}
    virtual size_t length();
    virtual valp at(int i);
    virtual valp& atRef(int i);
    virtual valp call(std::string f, std::vector<valp> args);
    virtual std::string print();
    std::vector<valp> values;
};

struct ValueRange : public Value {
    ValueRange(int beg, int end, int step) : beg(beg), end(end) {
        if (step == 0) throw std::runtime_error("Can't have a step of 0");
        this->step = step;
    }
    virtual size_t length();
    virtual valp at(int i);
    virtual valp& atRef(int id);
    virtual std::string print();
    int beg, end, step;
};

// String
struct ValueStr : public Value {
    ValueStr(std::string v) : value(v) {}
    virtual valp binop(std::string op, valp r);
    virtual std::string getStr() { return value; }
    virtual std::string print();
    std::string value;
};

// Calls script function
struct ValueFunction : public Value {
    /* args = names of arguments
       body = function body */
    ValueFunction(std::vector<std::string> args, statp body) : args(args), body(body) {}
    virtual std::string print();
    std::vector<std::string> args;
    statp body;
};

// Calls native function
struct ValueNativeFunc : public Value  {
    /* f = native function wrapper, takes a list of values as arguments, returns any value */
    ValueNativeFunc(std::function<valp(std::vector<valp>)> f) : f(f) {}
    std::function<valp(std::vector<valp>)> f;
    virtual std::string print();
};

struct ValueExternBase {
    virtual void assign(valp r)=0;
    virtual valp get()=0;
};

// Reference to native variable
template <typename T>
struct ValueExtern : public Value, public ValueExternBase {
    ValueExtern(T& ref) : ref(ref) {}
    virtual void assign(valp r) override;
    virtual valp get() override;
    virtual std::string print() {
        std::string s = "externvalue<";
        s += typeid(T).name();
        s += ">";
        return s;
    }
    T& ref;
};