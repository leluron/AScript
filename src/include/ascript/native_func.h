#pragma once

#include <vector>
#include <functional>
#include <stdexcept>

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

