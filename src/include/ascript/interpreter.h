#pragma once

#include <string>

class Script {
public:
    // Loads script from path
    Script(std::string path);
    // Launches script
    void run();
    // Returns whether script has finished
    bool isOver();

    // Links reference to script variable
    template <typename T>
    void link(std::string name, T& ref) {
        variables->getRef(name) = valp(new ValueExtern<T>(ref));
    }
    // Links native function to script variable
    template <typename T>
    void linkFunction(std::string name, std::function<T> f) {
        // Create wrapper function that takes list of values and returns value
        variables->getRef(name) = valp(new ValueNativeFunc([&](auto a) {
            return call(f, a);
        }));
    }

private:
    void load(std::string path);
    // Executes statement s with context vars
    void exec(valp vars, statp s);
    // Evaluates expresison e with context vars
    valp eval(valp vars, expp e);
    valp eval1(valp vars, expp e);

    valp& evalRef(valp vars, expp lp);

    valp evalFunc(valp ctx, std::string f, std::vector<valp> args);

    // Current return value; null means not returning
    valp ret = nullptr;
    // Script variables
    valp variables = valp(new ValueMap({}));
    // AST to execute
    statp code;
    std::string source;
    std::string filename;
};