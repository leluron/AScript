#include <ascript/script.h>
#include <vector>

using namespace std;

valp Value::unop(string op) {
    throw runtime_error("Unsupported Unop");
}
valp Value::binop(string op, valp r) {
    throw runtime_error("Unsupported Binop");
}
size_t Value::length() {
    throw runtime_error("Not iterable");
}
valp Value::at(int id) {
    throw runtime_error("Not iterable");
}
valp& Value::atRef(int id) {
    throw runtime_error("Not iterable");
}
valp Value::get(string mem) {
    throw runtime_error("Can't get member from non-map");
}
valp& Value::getRef(string mem) {
    throw runtime_error("Can't get member from non-map");
}
bool Value::isTrue() {
    throw runtime_error("Can't evaluate to boolean");
}
int Value::getInt() {
    throw runtime_error("Not an int");
}
valp Value::call(string f, vector<valp> args) {
    throw runtime_error("Can't call function from this value");
}
string Value::getStr() {
    throw runtime_error("Not a string");
}
valp ValueMap::get(std::string mem) { 
    return vars[mem];
}
valp &ValueMap::getRef(std::string mem) { 
    auto it = vars.find(mem);
    if (it == vars.end()) vars[mem] = valp(new ValueNone());
    return vars[mem];
}
size_t ValueList::length() {
    return values.size();
}
valp ValueList::at(int i) {
    return values.at(i);
}
valp& ValueList::atRef(int i) {
    if (i >= length()) values.resize(i+1);
    return values.at(i);
}
valp ValueList::call(std::string f, std::vector<valp> args) {
    if (f == "length" && args.size() == 0) return valp(new ValueInt(length()));
    throw std::runtime_error("Unknown method");
}
size_t ValueRange::length() {
    return ((end-beg)/step)+1;
}
valp ValueRange::at(int i) {
    return valp(new ValueInt(beg + step*i));
}
valp& ValueRange::atRef(int id) {
    throw std::runtime_error("Can't access range as left-value");
}

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
    if (op == "==") return l==r;
    if (op == "!=") return l!=r;
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