/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */
#include <map>
#include <cctype>
#include <iostream>
#include <string>
#include "exp.h"
#include "parser.h"
#include "program.h"
#include "../StanfordCPPLib/error.h"
#include "../StanfordCPPLib/tokenscanner.h"

#include "../StanfordCPPLib/simpio.h"
#include "../StanfordCPPLib/strlib.h"

using namespace std;

/* Function prototypes */

void processLine(string line, Program &program, EvalState &state);

/* Main program */
map<int, string, less<> > mymap;

void list();

void run();

void help();

bool cmp(int, int, char);

void input(string);

void if_then(string);

void go_to(string);

//
bool isnum(char c) {
    return (c >= '0' && c <= '9');
}

bool iskey(string token) {
    if (token == "LET")return false;
    if (token == "PRINT")return false;
    if(token=="INPUT")return false;
    return true;
}

EvalState state;
Program program;
auto it = mymap.begin();
bool ended=false;
int main() {

    while (true) {
        try {
            string input = getLine();
            if (isnum(input[0])) {
                int linenum = 0;
                int i = 0;
                while (input[i] != ' ') {
                    linenum = 10 * linenum + input[i++] - '0';
                    if (i == input.length())break;
                }
                if (i == input.length()) {
                    mymap.erase(linenum);
                }else {
                    input = input.substr(i+1);mymap[linenum] = input;
                }

            } else {
                if (!(input == "CLEAR" || input == "LIST" || input == "RUN" || input == "QUIT" || input == "HELP"))
                    processLine(input,program,state);
                if (input == "CLEAR"){mymap.clear();program.clear();state.stateClear();}
                if (input == "LIST")list();
                if (input == "RUN"){ended=false;run();}
                if (input == "QUIT")return 0;
                if (input == "HELP")help();

            }

        } catch (ErrorException &ex) {
            cout << ex.getMessage() << endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(string line, Program &program1, EvalState &state1) {
    if(line=="END"){ended=true;return;}
    int pos = 0;
    while (line[pos++] != ' ') { ; }
    string command = line.substr(0, pos - 1);
    line = line.substr(pos);
    if (command == "LET") {
        int i=0;
        while(line[i++]!=' ');
        if(!iskey(line.substr(0,i-1)))error("SYNTAX ERROR");
        TokenScanner scanner;
        scanner.ignoreWhitespace();
        scanner.scanNumbers();
        scanner.setInput(line);
        Expression *exp = parseExp(scanner);
        int value = exp->eval(state1);
        delete exp;
    }else {
        if (command == "INPUT"){input(line);}
        else {
            if (command == "IF") {
                if_then(line);

            } else {
                if (command == "PRINT") {
                    try {
                        TokenScanner scanner;
                        scanner.ignoreWhitespace();
                        scanner.scanNumbers();
                        scanner.setInput(line);
                        Expression *exp = parseExp(scanner);
                        int value = exp->eval(state1);
                        cout << value << endl;
                        delete exp;
                    }catch(ErrorException &ex){cout<<ex.getMessage()<<endl; }
                } else {
                    if (command == "GOTO") {
                        go_to(line);
                    }else{if(command=="REM"){;}
                        else{error("SYNTAX ERROR");}
                    }
                }
            }
        }
    }
}
    void list() {
    for (it = mymap.begin(); it != mymap.end(); it++) {
        cout << it->first << ' ' << it->second << endl;
    }
}

void run() {
    for (it = mymap.begin(); it != mymap.end(); it++) {
        string statement = it->second;
        if (statement.substr(0, 3) == "REM")continue;
        if (statement == "END"||it==mymap.end()--)break;
        processLine(statement, program, state);
        if(ended)return;
    }
}

void help() {
    cout << "EDGnb!" << endl;
}

void input(string s) {

    string number;
    bool end=true;bool sign = true;
    while(end) {
        cout << " ? ";
        number=getLine();//todo:may syntax_error here(input characters)
        end=false;
        sign=true;
        if (number[0] == '-') {
            number = number.substr(1);
            sign = false;
        }
        for (auto x: number) {
            if (x < '0' || x > '9') {
                cout<<"INVALID NUMBER"<<endl;
                end=true;
                break;
            }
        }
    }
    if(!sign)number="0-"+number;
    s = "LET " + s + " = " + number;
    processLine(s, program, state);
}

void go_to(string s) {
    int num = 0;
    for (auto x: s) {
        num = num * 10 + x - '0';
    }

    if (mymap.count(num)==0)cout << "LINE NUMBER ERROR" << endl;
    else {
        it = mymap.find(num);
        string str = it->second;

        processLine(str, program, state);
    }
}

void if_then(string s) {
    string linenum;
    string expr;
    int pos = 0;
    while (s.substr(pos++, 4) != "THEN") { ; }
    linenum = s.substr(pos + 4);
    expr = s.substr(0, pos - 2);
    string ls, rs;
    char op;
    pos = 0;
    while (expr[pos] != '<' && expr[pos] != '=' && expr[pos] != '>') { pos++; }
    ls = expr.substr(0, pos - 1);
    pos++;
    op = expr[pos - 1];
    rs = expr.substr(pos + 1);
    int lv, rv;
    TokenScanner scannerl, scannerr;
    scannerl.ignoreWhitespace();
    scannerl.scanNumbers();
    scannerl.setInput(ls);
    Expression *expl = parseExp(scannerl);
    lv = expl->eval(state);
    scannerr.ignoreWhitespace();
    scannerr.scanNumbers();
    scannerr.setInput(rs);
    Expression *exp = parseExp(scannerr);
    rv = exp->eval(state);
    delete exp;
    delete expl;
    if (cmp(lv, rv, op))go_to(linenum);
}

bool cmp(int v1, int v2, char op) {
    if (op == '=')return (v1 == v2);
    if (op == '<')return (v1 < v2);
    if (op == '>')return (v1 > v2);
}
