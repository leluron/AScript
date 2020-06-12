#include <ascript/script.h>
#include <iostream>
#include <experimental/filesystem>
#include <fstream>

using namespace std;


int main(void) {

    ofstream log("test_log");
    
    size_t num_tests = 0;
    size_t passed_tests = 0;
    for (auto& de : experimental::filesystem::directory_iterator("tests/scripts")) {
        auto p = de.path();
        num_tests += 1;
        Script script(p);
        try {
            script.run();
            passed_tests += 1;
            cout << "\033[30;42m" << p << "\033[0m" << endl;
        } catch (exception &e) {
            log << p << ":" << e.what() << endl;
            cout << "\033[30;41m" << p << "\033[0m" << endl;
        }
    }

    auto p = "tests/linking/test1.as";
    Script script(p);
    auto f = [](int x, int y){ return x-y; };
    int a;
    int x = 10;
    int y = 4;
    script.link("a", a);
    script.link("x", x);
    script.link("y", y);
    script.linkFunction<int(int, int)>("f", f);
    try {
        script.run();
        if (a != f(x,y)) throw runtime_error("Extern variables didn't link successfully");
        passed_tests += 1;
        cout << "\033[30;42m" << p << "\033[0m" << endl;
    } catch (exception &e) {
        log << p << ":" << e.what() << endl;
        cout << "\033[30;41m" << p << "\033[0m" << endl;
    }
    num_tests += 1;

    cout << passed_tests << "/" << num_tests << " tests passed" << endl;
}