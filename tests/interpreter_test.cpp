#include "interpreter.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "expr.h"
#include "loxvalue.h"
#include "runtimeerror.h"
#include "token.h"
#include "tokentype.h"

// ============================================================
// Test fixture — pure expression evaluation only
// ============================================================

class InterpreterTest : public ::testing::Test {
 protected:
  interpreter::Interpreter interp;

  Lexeme::Token tok(Lexeme::TokenType t, const std::string& lex) {
    return Lexeme::Token(t, lex, std::monostate{}, 1);
  }

  LoxValueType eval(syntax::Expr* expr) { return interp.evaluate(expr); }
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

TEST_F(InterpreterTest, UnaryBangFalse) {
  auto inner =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(false));
  syntax::Unary u(tok(Lexeme::TokenType::BANG, "!"), std::move(inner));
  EXPECT_EQ(eval(&u).get<bool>(), true);
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
// Grouping
// ============================================================

TEST_F(InterpreterTest, Grouping) {
  auto inner =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(42.0));
  syntax::Grouping g(std::move(inner));
  EXPECT_DOUBLE_EQ(eval(&g).get<double>(), 42.0);
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
// Logical operators
// ============================================================

static Lexeme::Token logicalTok(Lexeme::TokenType t, const std::string& lex) {
  return Lexeme::Token(t, lex, std::monostate{}, 1);
}

TEST_F(InterpreterTest, LogicalOrTrueShortCircuit) {
  auto left =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(true));
  auto right =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(false));
  auto orExpr = std::make_unique<syntax::Logical>(
      std::move(left), logicalTok(Lexeme::TokenType::OR, "or"),
      std::move(right));
  EXPECT_TRUE(eval(orExpr.get()).get<bool>());
}

TEST_F(InterpreterTest, LogicalOrFalseEvaluatesRight) {
  auto left =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(false));
  auto right =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(true));
  auto orExpr = std::make_unique<syntax::Logical>(
      std::move(left), logicalTok(Lexeme::TokenType::OR, "or"),
      std::move(right));
  EXPECT_TRUE(eval(orExpr.get()).get<bool>());
}

TEST_F(InterpreterTest, LogicalOrBothFalse) {
  auto left =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(false));
  auto right =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(false));
  auto orExpr = std::make_unique<syntax::Logical>(
      std::move(left), logicalTok(Lexeme::TokenType::OR, "or"),
      std::move(right));
  EXPECT_FALSE(eval(orExpr.get()).get<bool>());
}

TEST_F(InterpreterTest, LogicalAndBothTrue) {
  auto left =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(true));
  auto right =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(true));
  auto andExpr = std::make_unique<syntax::Logical>(
      std::move(left), logicalTok(Lexeme::TokenType::AND, "and"),
      std::move(right));
  EXPECT_TRUE(eval(andExpr.get()).get<bool>());
}

TEST_F(InterpreterTest, LogicalAndFalseShortCircuit) {
  auto left =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(false));
  auto right =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(true));
  auto andExpr = std::make_unique<syntax::Logical>(
      std::move(left), logicalTok(Lexeme::TokenType::AND, "and"),
      std::move(right));
  EXPECT_FALSE(eval(andExpr.get()).get<bool>());
}

TEST_F(InterpreterTest, LogicalAndReturnsRightValue) {
  auto left =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(1.0));
  auto right =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(2.0));
  auto andExpr = std::make_unique<syntax::Logical>(
      std::move(left), logicalTok(Lexeme::TokenType::AND, "and"),
      std::move(right));
  EXPECT_DOUBLE_EQ(eval(andExpr.get()).get<double>(), 2.0);
}

TEST_F(InterpreterTest, LogicalOrReturnsLeftValue) {
  auto left = std::make_unique<syntax::Literal>(
      syntax::Literal::LiteralValue(std::string("hello")));
  auto right =
      std::make_unique<syntax::Literal>(syntax::Literal::LiteralValue(2.0));
  auto orExpr = std::make_unique<syntax::Logical>(
      std::move(left), logicalTok(Lexeme::TokenType::OR, "or"),
      std::move(right));
  EXPECT_EQ(eval(orExpr.get()).get<std::string>(), "hello");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
