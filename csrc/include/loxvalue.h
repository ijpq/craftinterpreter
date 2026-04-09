#pragma once
#include "helper/object.h"
#include <string>
class Function;
using LoxValueType = Object<double, std::string, bool, std::monostate, Function*>;
