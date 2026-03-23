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
  	return prefix_expr(std::string{expr->op.lexeme}, {expr->left, expr->right});
  }
  Visitor::ReturnType ASTPrinter::visitGroupingExpr(Grouping *expr)  {
  	return prefix_expr("group", {expr->expression});
  }
  Visitor::ReturnType ASTPrinter::visitLiteralExpr(Literal *expr)  {
  	return Lexeme::literal_to_str(expr->literal);
  }
  Visitor::ReturnType ASTPrinter::visitUnaryExpr(Unary* expr)  {
  	return prefix_expr(std::string{expr->op.lexeme}, {expr->right});
  }


 std::string ASTPrinter::print(Expr* expr) {
 	return expr->accept(this);
 }

}

int main(int argc, char* argv[]) {
	using namespace syntax;
	auto minus_token = Lexeme::Token(Lexeme::TokenType::MINUS, "-", std::monostate{}, 1);
	auto literal_123 = std::make_shared<Literal>(Lexeme::Literal("123"));
	auto star = Lexeme::Token(Lexeme::TokenType::STAR, "*", std::monostate{}, 1);
	auto literal_4567 = std::make_shared<Literal>(Lexeme::Literal("45.67"));
	auto unary_minus = std::make_shared<Unary>(minus_token, literal_123.get());
	auto grouping = std::make_shared<Grouping>(literal_4567.get());
	Binary expression = Binary(
			unary_minus.get(), star, grouping.get()
		);
	auto printer = ASTPrinter();
	std::cout << printer.print(&expression) << std::endl;

}
