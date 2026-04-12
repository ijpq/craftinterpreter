package lox;

import java.util.List;

/** Chapter 7 Interpreter 单元测试 无需 JUnit，直接运行: java -cp . lox.InterpreterTest */
public class InterpreterTest {
  private static int passed = 0;
  private static int failed = 0;

  // ── 断言工具 ──────────────────────────────────────────────────────────────

  static void assertEqual(String testName, Object expected, Object actual) {
    if (expected == null ? actual == null : expected.equals(actual)) {
      System.out.println("  ✓ " + testName);
      passed++;
    } else {
      System.out.println("  ✗ " + testName);
      System.out.println("      expected: " + expected);
      System.out.println("      actual:   " + actual);
      failed++;
    }
  }

  static void assertRuntimeError(String testName, Runnable block) {
    try {
      block.run();
      System.out.println("  ✗ " + testName + " (expected RuntimeError, got none)");
      failed++;
    } catch (RuntimeError e) {
      System.out.println("  ✓ " + testName + " → RuntimeError: " + e.getMessage());
      passed++;
    }
  }

  // ── AST 构造辅助 ──────────────────────────────────────────────────────────

  static Token tok(TokenType type, String lexeme) {
    return new Token(type, lexeme, null, 1);
  }

  static Expr.Literal lit(Object value) {
    return new Expr.Literal(value);
  }

  static Expr.Unary unary(TokenType op, Expr right) {
    return new Expr.Unary(tok(op, op.toString()), right);
  }

  static Expr.Binary binary(Expr left, TokenType op, Expr right) {
    return new Expr.Binary(left, tok(op, op.toString()), right);
  }

  static Expr.Grouping group(Expr inner) {
    return new Expr.Grouping(inner);
  }

  // ── 辅助：运行完整 Lox 程序，捕获 stdout ────────────────────────────────

  static String runProgram(String source) {
    Lox.hadError = false;
    Lox.hadRuntimeError = false;
    java.io.ByteArrayOutputStream baos = new java.io.ByteArrayOutputStream();
    java.io.PrintStream oldOut = System.out;
    System.setOut(new java.io.PrintStream(baos));
    try {
      Scanner scanner = new Scanner(source);
      List<Token> tokens = scanner.scanTokens();
      Parser parser = new Parser(tokens);
      List<Stmt> stmts = parser.parse();
      if (!Lox.hadError) {
        new Interpreter().interpret(stmts);
      }
    } finally {
      System.setOut(oldOut);
    }
    return baos.toString().trim();
  }

  // ── 测试入口 ──────────────────────────────────────────────────────────────

  public static void main(String[] args) {
    Interpreter interp = new Interpreter();

    // ── 1. Literals ──────────────────────────────────────────────────────────
    System.out.println("\n[1] Literals");
    assertEqual("number", 42.0, interp.visitLiteralExpr(lit(42.0)));
    assertEqual("string", "hi", interp.visitLiteralExpr(lit("hi")));
    assertEqual("true", true, interp.visitLiteralExpr(lit(true)));
    assertEqual("false", false, interp.visitLiteralExpr(lit(false)));
    assertEqual("nil", null, interp.visitLiteralExpr(lit(null)));

    // ── 2. Grouping ──────────────────────────────────────────────────────────
    System.out.println("\n[2] Grouping");
    assertEqual("(42.0)", 42.0, interp.visitGroupingExpr(group(lit(42.0))));

    // ── 3. Unary ─────────────────────────────────────────────────────────────
    System.out.println("\n[3] Unary");
    assertEqual("-3", -3.0, interp.visitUnaryExpr(unary(TokenType.MINUS, lit(3.0))));
    assertEqual("!true", false, interp.visitUnaryExpr(unary(TokenType.BANG, lit(true))));
    assertEqual("!false", true, interp.visitUnaryExpr(unary(TokenType.BANG, lit(false))));
    assertEqual("!nil", true, interp.visitUnaryExpr(unary(TokenType.BANG, lit(null))));
    assertEqual(
        "!0", false, interp.visitUnaryExpr(unary(TokenType.BANG, lit(0.0)))); // 0 is truthy in Lox

    // ── 4. Arithmetic ────────────────────────────────────────────────────────
    System.out.println("\n[4] Binary Arithmetic");
    assertEqual("1+2", 3.0, interp.visitBinaryExpr(binary(lit(1.0), TokenType.PLUS, lit(2.0))));
    assertEqual("5-3", 2.0, interp.visitBinaryExpr(binary(lit(5.0), TokenType.MINUS, lit(3.0))));
    assertEqual("3*4", 12.0, interp.visitBinaryExpr(binary(lit(3.0), TokenType.STAR, lit(4.0))));
    assertEqual("10/4", 2.5, interp.visitBinaryExpr(binary(lit(10.0), TokenType.SLASH, lit(4.0))));

    // ── 5. String concatenation ──────────────────────────────────────────────
    System.out.println("\n[5] String Concatenation");
    assertEqual(
        "\"foo\"+\"bar\"",
        "foobar",
        interp.visitBinaryExpr(binary(lit("foo"), TokenType.PLUS, lit("bar"))));

    // ── 6. Comparison ────────────────────────────────────────────────────────
    System.out.println("\n[6] Comparison");
    assertEqual("3>2", true, interp.visitBinaryExpr(binary(lit(3.0), TokenType.GREATER, lit(2.0))));
    assertEqual(
        "2>3", false, interp.visitBinaryExpr(binary(lit(2.0), TokenType.GREATER, lit(3.0))));
    assertEqual(
        "3>=3", true, interp.visitBinaryExpr(binary(lit(3.0), TokenType.GREATER_EQUAL, lit(3.0))));
    assertEqual("2<3", true, interp.visitBinaryExpr(binary(lit(2.0), TokenType.LESS, lit(3.0))));
    assertEqual(
        "3<=3", true, interp.visitBinaryExpr(binary(lit(3.0), TokenType.LESS_EQUAL, lit(3.0))));

    // ── 7. Equality ──────────────────────────────────────────────────────────
    System.out.println("\n[7] Equality");
    assertEqual(
        "1==1", true, interp.visitBinaryExpr(binary(lit(1.0), TokenType.EQUAL_EQUAL, lit(1.0))));
    assertEqual(
        "1==2", false, interp.visitBinaryExpr(binary(lit(1.0), TokenType.EQUAL_EQUAL, lit(2.0))));
    assertEqual(
        "nil==nil",
        true,
        interp.visitBinaryExpr(binary(lit(null), TokenType.EQUAL_EQUAL, lit(null))));
    assertEqual(
        "nil==false",
        false,
        interp.visitBinaryExpr(binary(lit(null), TokenType.EQUAL_EQUAL, lit(false))));
    assertEqual(
        "1!=2", true, interp.visitBinaryExpr(binary(lit(1.0), TokenType.BANG_EQUAL, lit(2.0))));

    // ── 8. stringify ─────────────────────────────────────────────────────────
    System.out.println("\n[8] stringify");
    assertEqual("integer double → no .0", "42", interp.stringify(42.0));
    assertEqual("real double stays", "3.14", interp.stringify(3.14));
    assertEqual("nil → \"nil\"", "nil", interp.stringify(null));
    assertEqual("true → \"true\"", "true", interp.stringify(true));
    assertEqual("string passthrough", "hello", interp.stringify("hello"));

    // ── 9. Runtime Errors ────────────────────────────────────────────────────
    System.out.println("\n[9] Runtime Errors");
    assertRuntimeError(
        "- on string", () -> interp.visitUnaryExpr(unary(TokenType.MINUS, lit("oops"))));
    assertRuntimeError(
        "+ mixed types", () -> interp.visitBinaryExpr(binary(lit(1.0), TokenType.PLUS, lit("s"))));
    assertRuntimeError(
        "* on string", () -> interp.visitBinaryExpr(binary(lit("a"), TokenType.STAR, lit(2.0))));

    // ── Summary ──────────────────────────────────────────────────────────────
    System.out.println("");

    // ── 10. Functions / Return (Ch10) ────────────────────────────────────────
    System.out.println("[10] Functions and Return (Ch10)");

    assertEqual(
        "basic return value",
        "42",
        runProgram("fun add(a, b) { return a + b; } print add(20, 22);"));

    assertEqual(
        "no return stmt → nil",
        "nil",
        runProgram("fun f() {} print f();"));

    assertEqual(
        "early return",
        "yes",
        runProgram("fun sign(n) { if (n > 0) return \"yes\"; return \"no\"; } print sign(5);"));

    assertEqual(
        "recursive fibonacci",
        "13",
        runProgram(
            "fun fib(n) { if (n <= 1) return n; return fib(n-2) + fib(n-1); } print fib(7);"));

    assertEqual(
        "stringify LoxFunction",
        "<fn greet>",
        runProgram("fun greet(name) { return name; } print greet;"));

    assertRuntimeError(
        "call non-callable",
        () -> {
          Interpreter i2 = new Interpreter();
          Token paren = tok(TokenType.RIGHT_PAREN, ")");
          Expr.Call call =
              new Expr.Call(
                  new Expr.Literal(42.0), paren, java.util.Collections.emptyList());
          i2.visitCallExpr(call);
        });

    assertRuntimeError(
        "arity mismatch",
        () -> {
          Interpreter i2 = new Interpreter();
          Token fname = tok(TokenType.IDENTIFIER, "f");
          Token param = tok(TokenType.IDENTIFIER, "a");
          // fun f(a) { return a; }
          Stmt.Function decl =
              new Stmt.Function(
                  fname,
                  java.util.Arrays.asList(param),
                  java.util.Arrays.asList(
                      new Stmt.Return(
                          tok(TokenType.RETURN, "return"), new Expr.Variable(param))));
          i2.visitFunctionStmt(decl);
          // call f() with 0 arguments
          Expr.Call call =
              new Expr.Call(
                  new Expr.Variable(fname),
                  tok(TokenType.RIGHT_PAREN, ")"),
                  java.util.Collections.emptyList());
          i2.visitCallExpr(call);
        });

    // ── Summary ──────────────────────────────────────────────────────────────
    System.out.println("\n========================================");
    System.out.println("通过: " + passed + "  失败: " + failed);
    if (failed == 0) {
      System.out.println("所有测试通过 ✓");
    } else {
      System.out.println("部分测试失败 ✗");
      System.exit(1);
    }
  }
}
