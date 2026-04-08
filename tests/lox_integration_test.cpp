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
// Chapter 8 — variable assignment
// ============================================================

TEST(Ch8Assign, SimpleAssign) {
  EXPECT_EQ(runLox("var a = 1;\na = 2;\nprint a;"), "2\n");
}

TEST(Ch8Assign, AssignString) {
  EXPECT_EQ(runLox("var s = \"hello\";\ns = \"world\";\nprint s;"), "world\n");
}

TEST(Ch8Assign, AssignExpr) {
  EXPECT_EQ(runLox("var a = 1;\na = 2 + 3;\nprint a;"), "5\n");
}

TEST(Ch8Assign, AssignReturnsValue) {
  // assignment is an expression that returns the assigned value
  EXPECT_EQ(runLox("var a = 1;\nprint a = 2;"), "2\n");
}

TEST(Ch8Assign, AssignToNilVar) {
  EXPECT_EQ(runLox("var a;\na = 42;\nprint a;"), "42\n");
}

TEST(Ch8Assign, ChainedAssign) {
  EXPECT_EQ(runLox("var a = 1;\nvar b = 2;\na = b = 3;\nprint a;\nprint b;"),
            "3\n3\n");
}

TEST(Ch8Assign, UndefinedAssign) {
  std::string out = runLox("a = 1;");
  EXPECT_NE(out.find("Undefined"), std::string::npos)
      << "Expected 'Undefined' in output, got: " << out;
}

// ============================================================
// Chapter 8 — block scoping
// ============================================================

TEST(Ch8Block, BasicBlock) { EXPECT_EQ(runLox("{ print 1; }"), "1\n"); }

TEST(Ch8Block, VarInBlock) {
  EXPECT_EQ(runLox("{ var a = 10; print a; }"), "10\n");
}

TEST(Ch8Block, BlockScopeNotVisibleOutside) {
  std::string out = runLox("{ var a = 1; }\nprint a;");
  EXPECT_NE(out.find("Undefined"), std::string::npos)
      << "Expected 'Undefined' after block scope, got: " << out;
}

TEST(Ch8Block, OuterVarVisibleInBlock) {
  EXPECT_EQ(runLox("var a = \"outer\";\n{ print a; }"), "outer\n");
}

TEST(Ch8Block, ShadowOuterVar) {
  EXPECT_EQ(
      runLox("var a = \"outer\";\n{ var a = \"inner\"; print a; }\nprint a;"),
      "inner\nouter\n");
}

TEST(Ch8Block, AssignOuterVarInBlock) {
  EXPECT_EQ(runLox("var a = \"before\";\n{ a = \"after\"; }\nprint a;"),
            "after\n");
}

TEST(Ch8Block, NestedBlocks) {
  EXPECT_EQ(runLox("var a = 1;\n{ var b = 2;\n  { var c = 3;\n    print a + b "
                   "+ c; } }"),
            "6\n");
}

TEST(Ch8Block, EmptyBlock) { EXPECT_EQ(runLox("{ }"), ""); }

TEST(Ch8Block, MultipleStatements) {
  EXPECT_EQ(runLox("{\n  var a = 1;\n  var b = 2;\n  print a + b;\n}"), "3\n");
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
// Java vs C++ consistency helpers
// ============================================================

#ifndef LOX_JAVA_DIR
#error "LOX_JAVA_DIR must be defined via CMake compile definitions"
#endif

// Run lox source string through the Java interpreter, capture stdout+stderr
static std::string runJava(const std::string& source) {
  char tmpfile[] = "/tmp/lox_java_test_XXXXXX.lox";
  int fd = mkstemps(tmpfile, 4);
  if (fd < 0) return "[mkstemp failed]";
  write(fd, source.c_str(), source.size());
  close(fd);

  std::string cmd = "cd " + std::string(LOX_JAVA_DIR) + " && java -cp " +
                    std::string(LOX_JAVA_DIR) + " lox.Lox " + tmpfile + " 2>&1";

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

// Compare Java and C++ output for the same lox source
static void expectConsistent(const std::string& source) {
  std::string javaOut = runJava(source);
  std::string cppOut = runLox(source);
  EXPECT_EQ(javaOut, cppOut) << "Source: " << source << "\nJava:   " << javaOut
                             << "\nC++:    " << cppOut;
}

// ============================================================
// Chapter 9 — Java vs C++ consistency
// ============================================================

TEST(Ch9Consistency, IfTrue) { expectConsistent("if (true) print 1;"); }

TEST(Ch9Consistency, IfFalse) { expectConsistent("if (false) print 1;"); }

TEST(Ch9Consistency, IfElseTrue) {
  expectConsistent("if (true) print 1; else print 2;");
}

TEST(Ch9Consistency, IfElseFalse) {
  expectConsistent("if (false) print 1; else print 2;");
}

TEST(Ch9Consistency, IfNilFalsy) {
  expectConsistent("if (nil) print 1; else print 2;");
}

TEST(Ch9Consistency, IfZeroTruthy) {
  // 0 is truthy in Lox
  expectConsistent("if (0) print 1; else print 2;");
}

TEST(Ch9Consistency, LogicalOrTrue) {
  expectConsistent("print true or false;");
}

TEST(Ch9Consistency, LogicalOrFalse) {
  expectConsistent("print false or false;");
}

TEST(Ch9Consistency, LogicalAndTrue) {
  expectConsistent("print true and true;");
}

TEST(Ch9Consistency, LogicalAndFalse) {
  expectConsistent("print false and true;");
}

TEST(Ch9Consistency, LogicalOrReturnsValue) {
  // or returns the actual operand value, not a bool
  expectConsistent("print 1 or 2;");
}

TEST(Ch9Consistency, LogicalAndReturnsValue) {
  expectConsistent("print 1 and 2;");
}

TEST(Ch9Consistency, LogicalShortCircuitOr) {
  // left side truthy: right side not evaluated
  expectConsistent("var a = 0; true or (a = 1); print a;");
}

TEST(Ch9Consistency, LogicalShortCircuitAnd) {
  // left side falsy: right side not evaluated
  expectConsistent("var a = 0; false and (a = 1); print a;");
}

TEST(Ch9Consistency, IfWithBlock) {
  expectConsistent(
      "var a = 1;\nif (a == 1) { print \"yes\"; } else { print \"no\"; }");
}

// ============================================================
// Chapter 9 — while statement
// ============================================================

TEST(Ch9While, BasicWhile) {
  EXPECT_EQ(runLox("var i = 0;\nwhile (i < 3) { print i; i = i + 1; }"),
            "0\n1\n2\n");
}

TEST(Ch9While, WhileFalseNoExec) {
  EXPECT_EQ(runLox("while (false) { print 1; }"), "");
}

TEST(Ch9While, WhileCountdown) {
  EXPECT_EQ(runLox("var i = 3;\nwhile (i > 0) { print i; i = i - 1; }"),
            "3\n2\n1\n");
}

TEST(Ch9While, WhileAccumulate) {
  EXPECT_EQ(runLox("var sum = 0;\nvar i = 1;\n"
                   "while (i <= 5) { sum = sum + i; i = i + 1; }\nprint sum;"),
            "15\n");
}

// ============================================================
// Chapter 9 — for statement
// ============================================================

TEST(Ch9For, BasicFor) {
  EXPECT_EQ(runLox("for (var i = 0; i < 3; i = i + 1) print i;"), "0\n1\n2\n");
}

TEST(Ch9For, ForNoInit) {
  EXPECT_EQ(runLox("var i = 0;\nfor (; i < 3; i = i + 1) print i;"),
            "0\n1\n2\n");
}

TEST(Ch9For, ForNoIncrement) {
  EXPECT_EQ(runLox("for (var i = 0; i < 3;) { print i; i = i + 1; }"),
            "0\n1\n2\n");
}

TEST(Ch9For, ForAccumulate) {
  EXPECT_EQ(
      runLox("var sum = 0;\n"
             "for (var i = 1; i <= 10; i = i + 1) sum = sum + i;\nprint sum;"),
      "55\n");
}

TEST(Ch9For, ForFibonacci) {
  std::string src =
      "var a = 0;\n"
      "var temp;\n"
      "for (var b = 1; a < 10000; b = temp + b) {\n"
      "  print a;\n"
      "  temp = a;\n"
      "  a = b;\n"
      "}";
  EXPECT_EQ(runLox(src),
            "0\n1\n1\n2\n3\n5\n8\n13\n21\n34\n55\n89\n144\n233\n377\n610\n"
            "987\n1597\n2584\n4181\n6765\n");
}

// ============================================================
// Chapter 9 — Java vs C++ consistency (while / for)
// ============================================================

TEST(Ch9Consistency, WhileBasic) {
  expectConsistent("var i = 0;\nwhile (i < 3) { print i; i = i + 1; }");
}

TEST(Ch9Consistency, WhileAccumulate) {
  expectConsistent(
      "var sum = 0;\nvar i = 1;\n"
      "while (i <= 5) { sum = sum + i; i = i + 1; }\nprint sum;");
}

TEST(Ch9Consistency, ForBasic) {
  expectConsistent("for (var i = 0; i < 3; i = i + 1) print i;");
}

TEST(Ch9Consistency, ForFibonacci) {
  std::string src =
      "var a = 0;\n"
      "var temp;\n"
      "for (var b = 1; a < 10000; b = temp + b) {\n"
      "  print a;\n"
      "  temp = a;\n"
      "  a = b;\n"
      "}";
  expectConsistent(src);
}

// ============================================================

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
