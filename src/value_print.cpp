#include <ascript/script.h>
#include <sstream>

using namespace std;

string ValueInt::print() {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

string ValueFloat::print() {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

string ValueMap::print() {
    std::stringstream ss; 
    ss << "{";
    for (auto a : vars) {
        ss << a.first << ":" << a.second->print() << ";";
    }
    ss << "}";
    return ss.str();
}

string ValueList::print() {
    std::stringstream ss; 
    ss << "[";
    for (auto a : values) {
        ss << a->print() << ",";
    }
    ss << "]";
    return ss.str();
}

string ValueRange::print() {
    std::stringstream ss; 
    ss << "[" << beg << ".." << end;
    if (step != 1) ss << ".." << step;
    ss << "]";
    return ss.str();
}

string ValueStr::print() {
    std::stringstream ss;
    ss << "\"" << value << "\"";
    return ss.str();
}

string ValueFunction::print() {
    std::stringstream ss;
    ss << "function(" << args.size() << ")";
    return ss.str();
}

std::string ValueNativeFunc::print() {
    return "nativefunction";
}