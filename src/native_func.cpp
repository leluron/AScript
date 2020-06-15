#include <ascript/script.h>

using namespace std;

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