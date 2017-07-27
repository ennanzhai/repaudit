#ifndef _UTILITIES
#define _UTILITIES

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <cstdlib>
using namespace std;

enum ErrorCode
{
  OK,
  FILE_CANNOT_OPEN,
  LINE_ILLEGAL
};

// delete comment and return pure config string
string trimComment(string line);


#endif
