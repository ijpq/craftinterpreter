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
// Chapter 10 — functions and return
// ============================================================

TEST(Ch10Functions, BasicReturn) {
  EXPECT_EQ(runLox("fun add(a, b) { return a + b; } print add(3, 4);"), "7\n");
}

TEST(Ch10Functions, NoReturn) {
  EXPECT_EQ(runLox("fun f() {} print f();"), "nil\n");
}

TEST(Ch10Functions, EarlyReturn) {
  EXPECT_EQ(runLox("fun sign(n) { if (n > 0) return \"pos\"; return \"neg\"; }"
                   "print sign(5); print sign(-1);"),
            "pos\nneg\n");
}

TEST(Ch10Functions, ReturnInWhile) {
  EXPECT_EQ(runLox("fun firstOver(n) {"
                   "  var i = 0;"
                   "  while (true) { i = i + 1; if (i > n) return i; }"
                   "}"
                   "print firstOver(3);"),
            "4\n");
}

TEST(Ch10Functions, RecursiveFib) {
  EXPECT_EQ(
      runLox("fun fib(n) { if (n <= 1) return n; return fib(n-2)+fib(n-1); }"
             "print fib(7);"),
      "13\n");
}

TEST(Ch10Functions, StringifyFunction) {
  EXPECT_EQ(runLox("fun greet(name) { return name; } print greet;"),
            "<fn greet>\n");
}

TEST(Ch10Functions, ArityMismatch) {
  std::string out = runLox("fun f(a) { return a; } f();");
  EXPECT_NE(out.find("arguments"), std::string::npos)
      << "Expected arity error, got: " << out;
}

TEST(Ch10Functions, CallNonCallable) {
  std::string out = runLox("var x = 42; x();");
  EXPECT_NE(out.find("call"), std::string::npos)
      << "Expected 'can only call' error, got: " << out;
}

// ============================================================
// Chapter 10 — Java vs C++ consistency
// ============================================================

TEST(Ch10Consistency, BasicReturn) {
  expectConsistent("fun add(a, b) { return a + b; } print add(3, 4);");
}

TEST(Ch10Consistency, NoReturn) { expectConsistent("fun f() {} print f();"); }

TEST(Ch10Consistency, RecursiveFib) {
  expectConsistent(
      "fun fib(n) { if (n <= 1) return n; return fib(n-2)+fib(n-1); }"
      "print fib(7);");
}

TEST(Ch10Consistency, ReturnInWhile) {
  expectConsistent(
      "fun firstOver(n) {"
      "  var i = 0;"
      "  while (true) { i = i + 1; if (i > n) return i; }"
      "}"
      "print firstOver(3);");
}

TEST(Ch10Consistency, NestedFunctionDecl) {
  expectConsistent(
      "fun outer() {"
      "  fun inner() { return 42; }"
      "  return inner();"
      "}"
      "print outer();");
}

TEST(Ch10Consistency, NestedFunctionClosesOverOuter) {
  expectConsistent(
      "fun makeAdder(x) {"
      "  fun adder(y) { return x + y; }"
      "  return adder(10);"
      "}"
      "print makeAdder(5);");
}

TEST(Ch10Consistency, ArityMismatch) {
  expectConsistent("fun f(a) { return a; } f();");
}

TEST(Ch10Consistency, CallNonCallable) { expectConsistent("var x = 42; x();"); }

// ============================================================
// Chapter 11 — Resolver: variable resolution & closures
// ============================================================

TEST(Ch11Resolver, ClosureCapturesCorrectScope) {
  // The bug from ch11 intro: showA should always print "global"
  EXPECT_EQ(runLox("var a = \"global\";\n"
                   "{\n"
                   "  fun showA() {\n"
                   "    print a;\n"
                   "  }\n"
                   "  showA();\n"
                   "  var a = \"block\";\n"
                   "  showA();\n"
                   "}"),
            "global\nglobal\n");
}

TEST(Ch11Resolver, NestedScopeResolution) {
  EXPECT_EQ(runLox("var a = 1;\n"
                   "{\n"
                   "  var b = 2;\n"
                   "  {\n"
                   "    var c = 3;\n"
                   "    print a;\n"
                   "    print b;\n"
                   "    print c;\n"
                   "  }\n"
                   "}"),
            "1\n2\n3\n");
}

TEST(Ch11Resolver, ClosureKeepsEnv) {
  EXPECT_EQ(runLox("fun makeAdder(x) {\n"
                   "  fun add(y) { return x + y; }\n"
                   "  return add;\n"
                   "}\n"
                   "var addFive = makeAdder(5);\n"
                   "print addFive(10);"),
            "15\n");
}

TEST(Ch11Resolver, CounterClosure) {
  EXPECT_EQ(runLox("fun makeCounter() {\n"
                   "  var i = 0;\n"
                   "  fun count() {\n"
                   "    i = i + 1;\n"
                   "    print i;\n"
                   "  }\n"
                   "  return count;\n"
                   "}\n"
                   "var counter = makeCounter();\n"
                   "counter();\n"
                   "counter();\n"
                   "counter();"),
            "1\n2\n3\n");
}

TEST(Ch11Resolver, TopLevelReturnError) {
  std::string out = runLox("return 1;");
  EXPECT_NE(out.find("return"), std::string::npos)
      << "Expected top-level return error, got: " << out;
}

TEST(Ch11Resolver, SelfReferenceInInitializer) {
  std::string out = runLox(
      "var a = 1;\n"
      "{\n"
      "  var a = a;\n"
      "}");
  EXPECT_NE(out.find("own initializer"), std::string::npos)
      << "Expected self-reference error, got: " << out;
}

TEST(Ch11Resolver, DuplicateVarInScope) {
  std::string out = runLox(
      "{\n"
      "  var a = 1;\n"
      "  var a = 2;\n"
      "}");
  EXPECT_NE(out.find("Already"), std::string::npos)
      << "Expected duplicate var error, got: " << out;
}

TEST(Ch11Resolver, GlobalVarLookup) {
  // Global variables should still work without resolver registration
  EXPECT_EQ(runLox("var x = 42;\n"
                   "print x;"),
            "42\n");
}

TEST(Ch11Resolver, GlobalAssignment) {
  EXPECT_EQ(runLox("var x = 1;\n"
                   "x = 2;\n"
                   "print x;"),
            "2\n");
}

TEST(Ch11Resolver, RecursionStillWorks) {
  EXPECT_EQ(
      runLox(
          "fun fib(n) { if (n <= 1) return n; return fib(n-2) + fib(n-1); }\n"
          "print fib(7);"),
      "13\n");
}

// ============================================================
// Chapter 11 — Java vs C++ consistency
// ============================================================

TEST(Ch11Consistency, ClosureCapturesCorrectScope) {
  expectConsistent(
      "var a = \"global\";\n"
      "{\n"
      "  fun showA() {\n"
      "    print a;\n"
      "  }\n"
      "  showA();\n"
      "  var a = \"block\";\n"
      "  showA();\n"
      "}");
}

TEST(Ch11Consistency, ClosureKeepsEnv) {
  expectConsistent(
      "fun makeAdder(x) {\n"
      "  fun add(y) { return x + y; }\n"
      "  return add;\n"
      "}\n"
      "var addFive = makeAdder(5);\n"
      "print addFive(10);");
}

TEST(Ch11Consistency, CounterClosure) {
  expectConsistent(
      "fun makeCounter() {\n"
      "  var i = 0;\n"
      "  fun count() {\n"
      "    i = i + 1;\n"
      "    print i;\n"
      "  }\n"
      "  return count;\n"
      "}\n"
      "var counter = makeCounter();\n"
      "counter();\n"
      "counter();\n"
      "counter();");
}

TEST(Ch11Consistency, NestedScopeResolution) {
  expectConsistent(
      "var a = 1;\n"
      "{\n"
      "  var b = 2;\n"
      "  {\n"
      "    var c = 3;\n"
      "    print a;\n"
      "    print b;\n"
      "    print c;\n"
      "  }\n"
      "}");
}

TEST(Ch11Consistency, GlobalAssignment) {
  expectConsistent(
      "var x = 1;\n"
      "x = 2;\n"
      "print x;");
}

TEST(Ch11Consistency, TopLevelReturnError) {
  // Both should produce error output containing "return"
  std::string java = runJava("return 1;");
  std::string cpp = runLox("return 1;");
  // Don't compare exact strings (format differs), just verify both error
  EXPECT_NE(java.find("return"), std::string::npos);
  EXPECT_NE(cpp.find("return"), std::string::npos);
}

TEST(Ch11Consistency, SelfReferenceInInitializer) {
  std::string java = runJava("var a = 1;\n{\n  var a = a;\n}");
  std::string cpp = runLox("var a = 1;\n{\n  var a = a;\n}");
  EXPECT_NE(java.find("own initializer"), std::string::npos);
  EXPECT_NE(cpp.find("own initializer"), std::string::npos);
}

TEST(Ch11Consistency, DuplicateVarInScope) {
  std::string java = runJava("{\n  var a = 1;\n  var a = 2;\n}");
  std::string cpp = runLox("{\n  var a = 1;\n  var a = 2;\n}");
  EXPECT_NE(java.find("Already"), std::string::npos);
  EXPECT_NE(cpp.find("Already"), std::string::npos);
}

// ============================================================

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
