obj = {
    x = 0
    init = function(x) {
        this.x = x
    }
}

obj.init(f(8,3))
c = obj.x

reverse = function(list) {
    l2 = []
    i = 0
    while i<list.length() {
        l2[i] = list[list.length()-1-i]
        i = i+1
    }
    return l2
}


list = [2, "random_test", obj]

len = list.length()
d = list[1]
list2 = reverse(list)

assert(len == 3)