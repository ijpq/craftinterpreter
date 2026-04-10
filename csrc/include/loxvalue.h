#pragma once
#include <memory>
#include <string>

#include "helper/object.h"
class LoxFunction;
class LoxCallable;
using LoxValueType = Object<double, std::string, bool, std::monostate,
                            std::shared_ptr<LoxCallable>>;
