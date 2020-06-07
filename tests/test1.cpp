#include <ascript/script.h>
#include <iostream>

using namespace std;

int f0(int a, int b) { return a*b; }

int main(void) {
    
    Script script("tests/scripts/test1.as");
    int c;
    script.link("c", c);
    auto f1 = [](int a, int b) {return a*b;};
    script.linkFunction<int(int,int)>("f", &f0);
    script.linkFunction<int(int,int)>("f", f1);
    script.run();
    cout << c << endl;

    cout << script.getVars().print() << endl;
}