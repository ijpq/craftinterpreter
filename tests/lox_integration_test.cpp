#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <string>

#ifndef LOX_BINARY
#error "LOX_BINARY must be defined via CMake compile definitions"
#endif

// ============================================================
// Subprocess helper
// ============================================================

static std::string getLoxDir() {
  std::string p = LOX_BINARY;
  auto pos = p.find_last_of('/');
  return (pos == std::string::npos) ? "." : p.substr(0, pos);
}

// Run lox source string through the binary, capture stdout+stderr
static std::string runLox(const std::string& source) {
  char tmpfile[] = "/tmp/lox_test_XXXXXX.lox";
  int fd = mkstemps(tmpfile, 4);  // 4 = strlen(".lox")
  if (fd < 0) return "[mkstemp failed]";
  write(fd, source.c_str(), source.size());
  close(fd);

  std::string dir = getLoxDir();
  std::string cmd;
#ifdef __APPLE__
  cmd = "DYLD_LIBRARY_PATH=" + dir + " " + std::string(LOX_BINARY) + " " +
        tmpfile + " 2>&1";
#else
  cmd = "LD_LIBRARY_PATH=" + dir + " " + std::string(LOX_BINARY) + " " +
        tmpfile + " 2>&1";
#endif

  FILE* pipe = popen(cmd.c_str(), "r");
  std::string result;
  if (pipe) {
    char buf[512];
    while (fgets(buf, sizeof(buf), pipe)) result += buf;
    pclose(pipe);
  }
  unlink(tmpfile);
  return result;
}

// ============================================================
// Chapter 7 — Arithmetic
// ============================================================

TEST(Ch7Arithmetic, Add) { EXPECT_EQ(runLox("print 1 + 2;"), "3\n"); }
TEST(Ch7Arithmetic, Subtract) { EXPECT_EQ(runLox("print 10 - 3;"), "7\n"); }
TEST(Ch7Arithmetic, Multiply) { EXPECT_EQ(runLox("print 2 * 4;"), "8\n"); }
TEST(Ch7Arithmetic, Divide) { EXPECT_EQ(runLox("print 10 / 4;"), "2.5\n"); }
TEST(Ch7Arithmetic, Precedence) {
  EXPECT_EQ(runLox("print 1 + 2 * 3;"), "7\n");
}
TEST(Ch7Arithmetic, Grouping) {
  EXPECT_EQ(runLox("print (1 + 2) * 3;"), "9\n");
}

// ============================================================
// Chapter 7 — Number formatting
// ============================================================

TEST(Ch7Numbers, Integer) { EXPECT_EQ(runLox("print 42;"), "42\n"); }
TEST(Ch7Numbers, LargeInteger) { EXPECT_EQ(runLox("print 1000;"), "1000\n"); }
TEST(Ch7Numbers, Float) { EXPECT_EQ(runLox("print 3.14;"), "3.14\n"); }
TEST(Ch7Numbers, SmallFloat) { EXPECT_EQ(runLox("print 0.5;"), "0.5\n"); }

// ============================================================
// Chapter 7 — String concatenation
// ============================================================

TEST(Ch7Strings, Concat) {
  EXPECT_EQ(runLox("print \"hello\" + \" world\";"), "hello world\n");
}
TEST(Ch7Strings, ConcatEmpty) {
  EXPECT_EQ(runLox("print \"foo\" + \"bar\";"), "foobar\n");
}

// ============================================================
// Chapter 7 — Comparison
// ============================================================

TEST(Ch7Comparison, LessThan) { EXPECT_EQ(runLox("print 1 < 2;"), "true\n"); }
TEST(Ch7Comparison, GreaterThan) {
  EXPECT_EQ(runLox("print 2 > 1;"), "true\n");
}
TEST(Ch7Comparison, LessEqual) { EXPECT_EQ(runLox("print 1 <= 1;"), "true\n"); }
TEST(Ch7Comparison, GreaterFalse) {
  EXPECT_EQ(runLox("print 2 >= 3;"), "false\n");
}

// ============================================================
// Chapter 7 — Equality
// ============================================================

TEST(Ch7Equality, NumEqual) { EXPECT_EQ(runLox("print 1 == 1;"), "true\n"); }
TEST(Ch7Equality, NumNotEqual) {
  EXPECT_EQ(runLox("print 1 == 2;"), "false\n");
}
TEST(Ch7Equality, NilNil) { EXPECT_EQ(runLox("print nil == nil;"), "true\n"); }
TEST(Ch7Equality, NilFalse) {
  EXPECT_EQ(runLox("print nil == false;"), "false\n");
}
TEST(Ch7Equality, BoolTrue) {
  EXPECT_EQ(runLox("print true == true;"), "true\n");
}
TEST(Ch7Equality, BoolFalse) {
  EXPECT_EQ(runLox("print true == false;"), "false\n");
}
TEST(Ch7Equality, StringEqual) {
  EXPECT_EQ(runLox("print \"a\" == \"a\";"), "true\n");
}
TEST(Ch7Equality, StringNotEqual) {
  EXPECT_EQ(runLox("print \"a\" == \"b\";"), "false\n");
}
TEST(Ch7Equality, BangEqual) { EXPECT_EQ(runLox("print 1 != 2;"), "true\n"); }

// ============================================================
// Chapter 7 — Unary
// ============================================================

TEST(Ch7Unary, Minus) { EXPECT_EQ(runLox("print -5;"), "-5\n"); }
TEST(Ch7Unary, BangTrue) { EXPECT_EQ(runLox("print !true;"), "false\n"); }
TEST(Ch7Unary, BangFalse) { EXPECT_EQ(runLox("print !false;"), "true\n"); }
TEST(Ch7Unary, BangNil) { EXPECT_EQ(runLox("print !nil;"), "true\n"); }
TEST(Ch7Unary, BangZeroFalsy) {
  // 0 is truthy in Lox (only nil and false are falsy)
  EXPECT_EQ(runLox("print !0;"), "false\n");
}
TEST(Ch7Unary, Negate) { EXPECT_EQ(runLox("print -(1 + 2);"), "-3\n"); }

// ============================================================
// Chapter 7 — Literals
// ============================================================

TEST(Ch7Literals, True) { EXPECT_EQ(runLox("print true;"), "true\n"); }
TEST(Ch7Literals, False) { EXPECT_EQ(runLox("print false;"), "false\n"); }
TEST(Ch7Literals, Nil) { EXPECT_EQ(runLox("print nil;"), "nil\n"); }

// ============================================================
// Chapter 7 — Runtime errors
// ============================================================

TEST(Ch7RuntimeError, MinusOnString) {
  std::string out = runLox("-\"oops\";");
  EXPECT_NE(out.find("Operand"), std::string::npos)
      << "Expected runtime error, got: " << out;
}

TEST(Ch7RuntimeError, PlusMixedTypes) {
  std::string out = runLox("1 + \"s\";");
  EXPECT_NE(out.find("Operands"), std::string::npos)
      << "Expected runtime error, got: " << out;
}

TEST(Ch7RuntimeError, StarOnString) {
  std::string out = runLox("\"a\" * 2;");
  EXPECT_NE(out.find("Operand"), std::string::npos)
      << "Expected runtime error, got: " << out;
}

// ============================================================
// Chapter 8 — print statement
// ============================================================

TEST(Ch8Print, Number) { EXPECT_EQ(runLox("print 42;"), "42\n"); }
TEST(Ch8Print, String) { EXPECT_EQ(runLox("print \"hello\";"), "hello\n"); }
TEST(Ch8Print, Bool) { EXPECT_EQ(runLox("print true;"), "true\n"); }
TEST(Ch8Print, Nil) { EXPECT_EQ(runLox("print nil;"), "nil\n"); }
TEST(Ch8Print, Expr) { EXPECT_EQ(runLox("print 1 + 2;"), "3\n"); }
TEST(Ch8Print, Float) { EXPECT_EQ(runLox("print 3.14;"), "3.14\n"); }

TEST(Ch8Print, Multiple) {
  EXPECT_EQ(runLox("print 1;\nprint 2;\nprint 3;"), "1\n2\n3\n");
}

// ============================================================
// Chapter 8 — expression statement (no output)
// ============================================================

TEST(Ch8ExprStmt, NumberDiscarded) { EXPECT_EQ(runLox("1 + 2;"), ""); }
TEST(Ch8ExprStmt, StringDiscarded) { EXPECT_EQ(runLox("\"hello\";"), ""); }
TEST(Ch8ExprStmt, BoolDiscarded) { EXPECT_EQ(runLox("true;"), ""); }

// ============================================================
// Chapter 8 — var declaration + variable reference
// ============================================================

TEST(Ch8Var, DeclAndPrint) { EXPECT_EQ(runLox("var a = 1;\nprint a;"), "1\n"); }

TEST(Ch8Var, Arithmetic) {
  EXPECT_EQ(runLox("var b = 2;\nvar c = 3;\nprint b + c;"), "5\n");
}

TEST(Ch8Var, NilInitializer) { EXPECT_EQ(runLox("var d;\nprint d;"), "nil\n"); }

TEST(Ch8Var, StringVar) {
  EXPECT_EQ(runLox("var s = \"hello\";\nprint s;"), "hello\n");
}

TEST(Ch8Var, BoolVar) {
  EXPECT_EQ(runLox("var b = true;\nprint b;"), "true\n");
}

TEST(Ch8Var, MultipleVars) {
  EXPECT_EQ(runLox("var a = 1;\nvar b = 2;\nprint a + b;"), "3\n");
}

// ============================================================
// Chapter 8 — undefined variable → runtime error
// ============================================================

TEST(Ch8Var, UndefinedVar) {
  std::string out = runLox("print a;");
  EXPECT_NE(out.find("Undefined"), std::string::npos)
      << "Expected 'Undefined' in output, got: " << out;
}

// ============================================================
// Chapter 8 — comment
// ============================================================

TEST(Ch8Misc, CommentIgnored) {
  EXPECT_EQ(runLox("// this is a comment\nprint 1;"), "1\n");
}

TEST(Ch8Misc, InlineComment) {
  EXPECT_EQ(runLox("print 1; // inline comment"), "1\n");
}

// ============================================================

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
