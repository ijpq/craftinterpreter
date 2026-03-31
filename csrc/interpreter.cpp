#include "include/interpreter.h"

#include "include/expr.h"

namespace interpreter {
using syntax::stringify;
void Interpreter::interpret(syntax::Expr* expr) {
  LoxValueType v = evaluate(expr);
  std::cout << stringify(v) << std::endl;
}

// std::string stringify(LoxValueType object) {
//   if (object.hold_alternative<std::monostate>()) return "nil";
//   if (object.hold_alternative<bool>())
//     return object.get<bool>() ? "true" : "false";
//   if (object.hold_alternative<std::string>()) return
//   object.get<std::string>(); if (object.hold_alternative<double>()) {
//     double d = object.get<double>();
//     if (std::floor(d) == d && std::isfinite(d)) {
//       return std::to_string((long long)d);  // 42.0 -> "42"
//     }
//     char buf[32];
//     std::snprintf(buf, sizeof(buf), "%.14g", d);
//     return buf;  // 3.14 -> "3.14"
//   }
//   return "";
// }
}  // namespace interpreter