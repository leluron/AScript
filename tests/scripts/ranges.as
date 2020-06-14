i = 0
for i2 in [0 ..10 .. 2] {
    assert(i == i2)
    i += 2
}

assert(i == 12)

i = 0
for i2 in [0 ..10] {
    assert(i == i2)
    i += 1
}

assert(i == 11)