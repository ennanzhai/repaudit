// simple parser 
#ifndef _PARSER
#define _PARSER
#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <regex>

using namespace std;

class Parser{
private:
	vector<string> header, footer, statements;
	unordered_map<string, string> functionMap;
	//edit by Xi
	unordered_set<string> variableMap;
	bool isError;
	int lineNum;

	void init();
	/** coverting one line from input program to one statement in c++
	 *  The input line is trimmed (no comment in it)
	 **/
	void processLine(string line);
	/**
	 * trim line, delete comment, heading and trailing whitespaces
	 **/
	void trim(string& line);
	void trimWhitespaces(string& line);
	/**
	 * print parsing error, message
	 **/
	void printe(string s);
	void extractParameter(string line, vector<string>& parameters);
	void extractVariable(string left_line, string& variableName);
	void extractFunction(string right_line, string& functionName, vector<string>& parameters);
	void genStatement(string &lvariable, string& functionName, vector<string>& parameters);
public:
	bool fail(){return isError;};
	Parser(string filename);
	void output(string filename);

};

#endif
