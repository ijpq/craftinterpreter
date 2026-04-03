#include "interpreter.h"

#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "environment.h"
#include "expr.h"
#include "runtimeerror.h"
#include "stmt.h"
#include "token.h"
#include "tokentype.h"

// ============================================================
// Test fixture
// ============================================================

class InterpreterTest : public ::testing::Test {
 protected:
  interpreter::Interpreter interp;

  Lexeme::Token tok(Lexeme::TokenType t, const std::string& lex) {
    return Lexeme::Token(t, lex, std::monostate{}, 1);
  }
  Lexeme::Token identTok(const std::string& name) {
    return Lexeme::Token(Lexeme::TokenType::IDENTIFIER, name, std::monostate{},
                         1);
  }

  LoxValueType eval(syntax::Expr* expr) { return interp.evaluate(expr); }

  std::string exec(std::vector<std::unique_ptr<SST::Stmt>>& stmts) {
    std::ostringstream oss;
    std::streambuf* old = std::cout.rdbuf(oss.rdbuf());
    interp.interpret(stmts);
    std::cout.rdbuf(old);
    return oss.str();
  }
};

// ============================================================
// Literals
// ============================================================

TEST_F(InterpreterTest, LiteralNumber) {
  syntax::Literal lit(syntax::Literal::LiteralValue(42.0));
  EXPECT_DOUBLE_EQ(eval(&lit).get<double>(), 42.0);
}

TEST_F(InterpreterTest, LiteralString) {
  syntax::Literal lit(syntax::Literal::LiteralValue(std::string("hello")));
  EXPECT_EQ(eval(&lit).get<std::string>(), "hello");
}

TEST_F(InterpreterTest, LiteralBool) {
  syntax::Literal lit(syntax::Literal::LiteralValue(true));
  EXPECT_EQ(eval(&lit).get<bool>(), true);
}

TEST_F(InterpreterTest, LiteralNil) {
  syntax::Literal lit(syntax::Literal::LiteralValue(std::monostate{}));
  EXPECT_TRUE(eval(&lit).hold_alternative<std::monostate>());
}

// ============================================================
// Unary
// ============================================================

TEST_F(InterpreterTest, UnaryMinus) {
  auto inner =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(3.0));
  syntax::Unary u(tok(Lexeme::TokenType::MINUS, "-"), std::move(inner));
  EXPECT_DOUBLE_EQ(eval(&u).get<double>(), -3.0);
}

TEST_F(InterpreterTest, UnaryBangTrue) {
  auto inner =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(true));
  syntax::Unary u(tok(Lexeme::TokenType::BANG, "!"), std::move(inner));
  EXPECT_EQ(eval(&u).get<bool>(), false);
}

TEST_F(InterpreterTest, UnaryBangNil) {
  auto inner = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::monostate{}));
  syntax::Unary u(tok(Lexeme::TokenType::BANG, "!"), std::move(inner));
  EXPECT_EQ(eval(&u).get<bool>(), true);
}

TEST_F(InterpreterTest, UnaryBangZero) {
  auto inner =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(0.0));
  syntax::Unary u(tok(Lexeme::TokenType::BANG, "!"), std::move(inner));
  EXPECT_EQ(eval(&u).get<bool>(), false);
}

// ============================================================
// Binary arithmetic
// ============================================================

TEST_F(InterpreterTest, BinaryAdd) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(1.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(2.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::PLUS, "+"),
                   std::move(r));
  EXPECT_DOUBLE_EQ(eval(&b).get<double>(), 3.0);
}

TEST_F(InterpreterTest, BinarySubtract) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(5.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(3.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::MINUS, "-"),
                   std::move(r));
  EXPECT_DOUBLE_EQ(eval(&b).get<double>(), 2.0);
}

TEST_F(InterpreterTest, BinaryMultiply) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(3.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(4.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::STAR, "*"),
                   std::move(r));
  EXPECT_DOUBLE_EQ(eval(&b).get<double>(), 12.0);
}

TEST_F(InterpreterTest, BinaryDivide) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(10.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(4.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::SLASH, "/"),
                   std::move(r));
  EXPECT_DOUBLE_EQ(eval(&b).get<double>(), 2.5);
}

TEST_F(InterpreterTest, BinaryStringConcat) {
  auto l = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::string("foo")));
  auto r = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::string("bar")));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::PLUS, "+"),
                   std::move(r));
  EXPECT_EQ(eval(&b).get<std::string>(), "foobar");
}

// ============================================================
// Comparison
// ============================================================

TEST_F(InterpreterTest, Greater) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(3.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(2.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::GREATER, ">"),
                   std::move(r));
  EXPECT_EQ(eval(&b).get<bool>(), true);
}

TEST_F(InterpreterTest, EqualEqual) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(1.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(1.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::EQUAL_EQUAL, "=="),
                   std::move(r));
  EXPECT_EQ(eval(&b).get<bool>(), true);
}

TEST_F(InterpreterTest, NilEqualsNil) {
  auto l = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::monostate{}));
  auto r = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::monostate{}));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::EQUAL_EQUAL, "=="),
                   std::move(r));
  EXPECT_EQ(eval(&b).get<bool>(), true);
}

TEST_F(InterpreterTest, NilNotEqualFalse) {
  auto l = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::monostate{}));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(false));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::EQUAL_EQUAL, "=="),
                   std::move(r));
  EXPECT_EQ(eval(&b).get<bool>(), false);
}

// ============================================================
// Grouping
// ============================================================

TEST_F(InterpreterTest, Grouping) {
  auto inner =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(42.0));
  syntax::Grouping g(std::move(inner));
  EXPECT_DOUBLE_EQ(eval(&g).get<double>(), 42.0);
}

// ============================================================
// Additional Unary
// ============================================================

TEST_F(InterpreterTest, UnaryBangFalse) {
  auto inner =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(false));
  syntax::Unary u(tok(Lexeme::TokenType::BANG, "!"), std::move(inner));
  EXPECT_EQ(eval(&u).get<bool>(), true);
}

// ============================================================
// Additional Comparison
// ============================================================

TEST_F(InterpreterTest, Less) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(2.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(3.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::LESS, "<"),
                   std::move(r));
  EXPECT_EQ(eval(&b).get<bool>(), true);
}

TEST_F(InterpreterTest, GreaterEqual) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(3.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(3.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::GREATER_EQUAL, ">="),
                   std::move(r));
  EXPECT_EQ(eval(&b).get<bool>(), true);
}

TEST_F(InterpreterTest, LessEqual) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(3.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(3.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::LESS_EQUAL, "<="),
                   std::move(r));
  EXPECT_EQ(eval(&b).get<bool>(), true);
}

// ============================================================
// Runtime errors
// ============================================================

TEST_F(InterpreterTest, MinusOnStringThrows) {
  auto inner = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::string("oops")));
  syntax::Unary u(tok(Lexeme::TokenType::MINUS, "-"), std::move(inner));
  EXPECT_THROW(eval(&u), interpreter::InterpreterRuntimeError);
}

TEST_F(InterpreterTest, PlusMixedTypesThrows) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(1.0));
  auto r = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::string("s")));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::PLUS, "+"),
                   std::move(r));
  EXPECT_THROW(eval(&b), interpreter::InterpreterRuntimeError);
}

TEST_F(InterpreterTest, StarOnStringThrows) {
  auto l = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::string("a")));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(2.0));
  syntax::Binary b(std::move(l), tok(Lexeme::TokenType::STAR, "*"),
                   std::move(r));
  EXPECT_THROW(eval(&b), interpreter::InterpreterRuntimeError);
}

// ============================================================
// stringify
// ============================================================

TEST_F(InterpreterTest, StringifyInteger) {
  EXPECT_EQ(syntax::stringify(LoxValueType(42.0)), "42");
}

TEST_F(InterpreterTest, StringifyFloat) {
  EXPECT_EQ(syntax::stringify(LoxValueType(3.14)), "3.14");
}

TEST_F(InterpreterTest, StringifyNil) {
  EXPECT_EQ(syntax::stringify(LoxValueType(std::monostate{})), "nil");
}

TEST_F(InterpreterTest, StringifyTrue) {
  EXPECT_EQ(syntax::stringify(LoxValueType(true)), "true");
}

TEST_F(InterpreterTest, StringifyString) {
  EXPECT_EQ(syntax::stringify(LoxValueType(std::string("hello"))), "hello");
}

// ============================================================
// Chapter 8: print statement
// ============================================================

TEST_F(InterpreterTest, PrintStatement) {
  auto expr =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(42.0));
  std::vector<std::unique_ptr<SST::Stmt>> stmts;
  stmts.push_back(std::make_unique<SST::Print>(std::move(expr)));
  EXPECT_EQ(exec(stmts), "42\n");
}

TEST_F(InterpreterTest, PrintString) {
  auto expr = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::string("hello")));
  std::vector<std::unique_ptr<SST::Stmt>> stmts;
  stmts.push_back(std::make_unique<SST::Print>(std::move(expr)));
  EXPECT_EQ(exec(stmts), "hello\n");
}

TEST_F(InterpreterTest, PrintNil) {
  auto expr = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::monostate{}));
  std::vector<std::unique_ptr<SST::Stmt>> stmts;
  stmts.push_back(std::make_unique<SST::Print>(std::move(expr)));
  EXPECT_EQ(exec(stmts), "nil\n");
}

// ============================================================
// Chapter 8: var declaration + variable lookup
// ============================================================

TEST_F(InterpreterTest, VarDeclAndLookup) {
  Lexeme::Token name = identTok("a");
  auto init =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(42.0));
  auto printExpr = std::make_unique<syntax::Variable>(name);
  std::vector<std::unique_ptr<SST::Stmt>> stmts;
  stmts.push_back(std::make_unique<SST::Var>(name, std::move(init)));
  stmts.push_back(std::make_unique<SST::Print>(std::move(printExpr)));
  EXPECT_EQ(exec(stmts), "42\n");
}

TEST_F(InterpreterTest, VarDeclNilInitializer) {
  Lexeme::Token name = identTok("a");
  auto printExpr = std::make_unique<syntax::Variable>(name);
  std::vector<std::unique_ptr<SST::Stmt>> stmts;
  stmts.push_back(std::make_unique<SST::Var>(name, nullptr));
  stmts.push_back(std::make_unique<SST::Print>(std::move(printExpr)));
  EXPECT_EQ(exec(stmts), "nil\n");
}

TEST_F(InterpreterTest, UndefinedVariableThrows) {
  Lexeme::Token name = identTok("undeclared");
  syntax::Variable v(name);
  EXPECT_THROW(eval(&v), interpreter::InterpreterRuntimeError);
}

TEST_F(InterpreterTest, ExpressionStatementDiscardsValue) {
  auto l =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(1.0));
  auto r =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(2.0));
  auto expr = std::make_unique<syntax::Binary>(
      std::move(l), tok(Lexeme::TokenType::PLUS, "+"), std::move(r));
  std::vector<std::unique_ptr<SST::Stmt>> stmts;
  stmts.push_back(std::make_unique<SST::Expression>(std::move(expr)));
  EXPECT_EQ(exec(stmts), "");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
