#include <iostream>
#include <string>
#include <variant>

#include "../csrc/include/helper/object.h"

using namespace interpreter;
using namespace std;
int main() {
  Object<double, int, std::string> obj = std::string("1");
  std::cout << obj.get<std::string>() << std::endl;
  variant<double, int, std::string> v = 3.14;
  obj = v;
  std::cout << obj.get<double>() << std::endl;
}