#include <gtest/gtest.h>
#include "../asas_fixture.h"

TEST(FunctionTest, SimpleFunctionDeclarationAndCall) {
  const char *source =
      "func greet() {\n"
      "  print \"Hello, World!\";\n"
      "}\n"
      "greet();\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> Hello, World!\n");
}

TEST(FunctionTest, FunctionWithParameters) {
  const char *source =
      "func add(a, b) {\n"
      "  return a + b;\n"
      "}\n"
      "var result = add(3, 5);\n"
      "print result;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 8.00\n");
}

TEST(FunctionTest, FunctionWithReturnValue) {
  const char *source =
      "func square(n) {\n"
      "  return n * n;\n"
      "}\n"
      "var result = square(6);\n"
      "print result;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 36.00\n");
}

TEST(FunctionTest, NestedFunctionCalls) {
  const char *source =
      "func multiply(x, y) {\n"
      "  return x * y;\n"
      "}\n"
      "func square(n) {\n"
      "  return multiply(n, n);\n"
      "}\n"
      "var result = square(4);\n"
      "print result;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 16.00\n");
}

TEST(FunctionTest, FunctionWithoutReturn) {
  const char *source =
      "func greet(name) {\n"
      "  print \"Hello, \" + name + \"!\";\n"
      "}\n"
      "greet(\"Alice\");\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> Hello, Alice!\n");
}

TEST(FunctionTest, FunctionWithMultipleParameters) {
  const char *source =
      "func concatenate(str1, str2, str3) {\n"
      "  return str1 + str2 + str3;\n"
      "}\n"
      "var result = concatenate(\"Asas \", \"is \", \"fun!\");\n"
      "print result;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> Asas is fun!\n");
}

TEST(FunctionTest, FunctionCallWithExpressionArguments) {
  const char *source =
      "func add(a, b) {\n"
      "  return a + b;\n"
      "}\n"
      "var result = add(2 * 3, 4 + 5);\n"
      "print result;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 15.00\n");
}

TEST(FunctionTest, RecursiveFunction) {
  const char *source =
      "func factorial(n) {\n"
      "  if (n <= 1) return 1;\n"
      "  return n * factorial(n - 1);\n"
      "}\n"
      "var result = factorial(5);\n"
      "print result;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 120.00\n");
}

TEST(FunctionTest, FunctionWithVariableShadowing) {
  const char *source =
      "var x = 10;\n"
      "func printX() {\n"
      "  var x = 5;\n"
      "  print x;\n"
      "}\n"
      "printX();\n"
      "print x;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 5.00\n-> 10.00\n");
}

TEST(FunctionTest, TwoFunctionsCallingEachOther) {
  const char *source =
      "func foo() {\n"
      "  print \"In foo\";\n"
      "  bar();\n"
      "}\n"
      "func bar() {\n"
      "  print \"In bar\";\n"
      "}\n"
      "foo();\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> In foo\n-> In bar\n");
}

TEST(FunctionTest, TwoFunctionsWithSameNameInDifferentScopes) {
  const char *source =
      "func outer() {\n"
      "  func inner() {\n"
      "    print \"Inner function\";\n"
      "  }\n"
      "  inner();\n"
      "}\n"
      "func inner() {\n"
      "  print \"Outer function\";\n"
      "}\n"
      "outer();\n"
      "inner();\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> Inner function\n-> Outer function\n");
}

TEST(FunctionTest, TwoFunctionsWithSameLocalVariablesName) {
  const char *source =
      "func funcA() {\n"
      "  var x = 1;\n"
      "  print x;\n"
      "}\n"
      "func funcB() {\n"
      "  var x = 2;\n"
      "  print x;\n"
      "}\n"
      "funcA();\n"
      "funcB();\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 1.00\n-> 2.00\n");
}

TEST(FunctionTest, CallFunctionWithExpression) {
  const char *source =
      "func sum(a, b, c) {\n"
      "  return a + b + c;\n"
      "}\n"
      "var result = 10 + sum(1 + 2, 3 * 4, 5 - 1);\n"
      "print result;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 29.00\n");
}
