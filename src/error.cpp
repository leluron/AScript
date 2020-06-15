#include <ascript/script.h>
#include <string>
#include <sstream>

using namespace std;

string getLine(string str, int n) {
    stringstream ss(str);
    int lineNum = 1;
    string line;
    while (std::getline(ss, line) && lineNum < n) lineNum++;
    return line;
}

InterpreterError::InterpreterError(string filename, string source, SourceInfo srcinfo, const string &str) {
    stringstream ss;
    ss << filename << ":";
    if (srcinfo.line != -1) ss << srcinfo.line << ":" << srcinfo.column << ":";
    ss << "error: " << str << endl;
    if (srcinfo.line != -1) {
        auto line = getLine(source, srcinfo.line);
        ss << line << endl;
        for (int i=1;i<srcinfo.column;i++) ss << " ";
        ss << "^";
        int tildelen = srcinfo.end_index-srcinfo.start_index;
        int maxlen = line.length()-srcinfo.column;
        if (tildelen > maxlen) tildelen = maxlen;
        for (int i=0;i<tildelen;i++) ss << "~";
        ss << endl; 
    }
    this->str = ss.str();
}

const char* InterpreterError::what() const noexcept {
    return str.c_str();
}