#include "../include/utilities.hpp"


string trimComment(string line){
  //trim '#' and the following characters
  size_t pos = line.find('#');
  line = line.substr(0, pos);
  //trim heading spaces
  pos = line.find_first_not_of(" \t");
  if (pos != string::npos) {
    line = line.substr(pos);
  }else {
    line = "";
  }
  return line;
}
