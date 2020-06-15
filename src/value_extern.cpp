#include <ascript/script.h>
#include <vector>

using namespace std;

template<>
void ValueExtern<int>::assign(valp r) {
    if (auto rv = dynamic_pointer_cast<ValueInt>(r))
        ref = rv->value;
    else if (auto rv = dynamic_pointer_cast<ValueFloat>(r))
        ref = (int)rv->value;
    else throw runtime_error("Uncompatible types");
}

template<>
void ValueExtern<float>::assign(valp r) {
    if (auto rv = dynamic_pointer_cast<ValueInt>(r))
        ref = (float)rv->value;
    else if (auto rv = dynamic_pointer_cast<ValueFloat>(r))
        ref = rv->value;
    else throw runtime_error("Uncompatible types");
}

template<>
void ValueExtern<string>::assign(valp r) {
    if (auto rv = dynamic_pointer_cast<ValueStr>(r))
        ref = rv->value;
    else throw runtime_error("Uncompatible types");
}

template<>
valp ValueExtern<int>::get() {
    return valp(new ValueInt(ref));
}

template<>
valp ValueExtern<float>::get() {
    return valp(new ValueFloat(ref));
}

template<>
valp ValueExtern<string>::get() {
    return valp(new ValueStr(ref));
}