obj = {
    a = 2
    f = function() return this.a
}

assert(obj.f() == 2)