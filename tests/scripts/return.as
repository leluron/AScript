f = function() {
    return 2
    return 4
}

assert(f() == 2)


f = function() {
    for i in [0..4] {
        if i == 2 return i
        if i == 3 return i
    }
}

assert(f() == 2)