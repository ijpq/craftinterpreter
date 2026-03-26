#include "include/expr.h"
#include "include/token.h"
#include "include/tokentype.h"
#include <variant>
namespace syntax {

  std::string ASTPrinter::prefix_expr(std::string name, std::initializer_list<Expr*> exprs) {
  	std::string res;
  	res += "(" + name;
  	for (auto&& expr : exprs) 
  		res += " " + expr->accept(this); 

  	res += ")";
  	return res;
  }
  Visitor::ReturnType ASTPrinter::visitBinaryExpr(Binary *expr)  {
  	return prefix_expr(std::string{expr->op.lexeme}, {expr->left.get(), expr->right.get()});
  }
  Visitor::ReturnType ASTPrinter::visitGroupingExpr(Grouping *expr)  {
  	return prefix_expr("group", {expr->expression.get()});
  }
  std::string literalval_to_str(Literal* literal) {
  		return std::visit([](auto &&args) -> std::string {
  			using T = std::decay_t<decltype(args)>;
  			if constexpr (std::is_same_v<T, bool>) {
  					return args ? "true" : "false";  
  			} else if constexpr (std::is_same_v<T, std::monostate>) {
  				return "nil";
  			} else if constexpr (std::is_same_v<T, std::string>) {
  				return args;
  			} else if constexpr (std::is_same_v<T, double>) {
  					return Lexeme::double_to_string(args);
  			}
  		}, literal->literal);
  }
  Visitor::ReturnType ASTPrinter::visitLiteralExpr(Literal *expr)  {
  	return literalval_to_str(expr);
  }
  Visitor::ReturnType ASTPrinter::visitUnaryExpr(Unary* expr)  {
  	return prefix_expr(std::string{expr->op.lexeme}, {expr->right.get()});
  }


 std::string ASTPrinter::print(Expr* expr) {
 	return expr->accept(this);
 }

}

int main(int argc, char* argv[]) {
	using namespace syntax;
	auto minus_token = Lexeme::Token(Lexeme::TokenType::MINUS, "-", std::monostate{}, 1);
	auto literal_123 = std::make_unique<Literal>(Literal("123"));
	auto star = Lexeme::Token(Lexeme::TokenType::STAR, "*", std::monostate{}, 1);
	auto literal_4567 = std::make_unique<Literal>(Literal("45.67"));
	auto unary_minus = std::make_unique<Unary>(minus_token, std::move(literal_123));
	auto grouping = std::make_unique<Grouping>(std::move(literal_4567));
	Binary expression = Binary(
			std::move(unary_minus), star, std::move(grouping)
		);
	auto printer = ASTPrinter();
	std::cout << printer.print(&expression) << std::endl;

}
