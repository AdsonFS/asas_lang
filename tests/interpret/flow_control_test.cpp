#include <gtest/gtest.h>
#include "../asas_fixture.h"

TEST(FlowControlTest, SimpleIfStatement) {
  const char *source =
      "var x = 10;\n"
      "if (x > 5) {\n"
      "  print \"x is greater than 5\";\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> x is greater than 5\n");
}

TEST(FlowControlTest, IfElseStatement) {
  const char *source =
      "var x = 3;\n"
      "if (x > 5) {\n"
      "  print \"x is greater than 5\";\n"
      "} else {\n"
      "  print \"x is not greater than 5\";\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> x is not greater than 5\n");
}

TEST(FlowControlTest, NestedIfElseStatement) {
  const char *source =
      "var x = 7;\n"
      "if (x > 10) {\n"
      "  print \"x is greater than 10\";\n"
      "} else if (x > 5) {\n"
      "  print \"x is greater than 5 but not greater than 10\";\n"
      "} else {\n"
      "  print \"x is 5 or less\";\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> x is greater than 5 but not greater than 10\n");
}

TEST(FlowControlTest, IfStatementWithoutBraces) {
  const char *source =
      "var x = 15;\n"
      "if (x > 10)\n"
      "  print \"x is greater than 10\";\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> x is greater than 10\n");
}

TEST(FlowControlTest, IfStatementConditionFalse) {
  const char *source =
      "var x = 2;\n"
      "if (x > 5) {\n"
      "  print \"This will not be printed\";\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "");
}

TEST(FlowControlTest, IfElseIfElseChain) {
  const char *source =
      "var score = 85;\n"
      "if (score >= 90) {\n"
      "  print \"Grade: A\";\n"
      "} else if (score >= 80) {\n"
      "  print \"Grade: B\";\n"
      "} else if (score >= 70) {\n"
      "  print \"Grade: C\";\n"
      "} else {\n"
      "  print \"Grade: F\";\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> Grade: B\n");
}

TEST(FlowControlTest, ifStatementScopeTest) {
  const char *source =
      "var x = 50;\n"
      "if (x > 20) {\n"
      "  var y = x + 30;\n"
      "  print y;\n"
      "}\n"
      "print x;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 80.00\n-> 50.00\n");
}

TEST(FlowControlTest, ForStatement) {
  const char *source =
      "for (var i = 0; i < 5; i = i + 1) {\n"
      "  print i;\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 0.00\n-> 1.00\n-> 2.00\n-> 3.00\n-> 4.00\n");
}

TEST(FlowControlTest, ForStatementWithComplexBody) {
  const char *source =
      "for (var i = 1; i <= 3; i = i + 1) {\n"
      "  var square = i * i;\n"
      "  print square;\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 1.00\n-> 4.00\n-> 9.00\n");
}

TEST(FlowControlTest, ForStatementWithMissingParts) {
  const char *source =
      "var count = 0;\n"
      "for (; count < 3;) {\n"
      "  print count;\n"
      "  count = count + 1;\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 0.00\n-> 1.00\n-> 2.00\n");
}

TEST(FlowControlTest, ForStatementWithVariableOutside) {
  const char *source =
      "var sum = 0;\n"
      "for (var i = 1; i <= 5; i = i + 1) {\n"
      "  sum = sum + i;\n"
      "}\n"
      "print sum;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 15.00\n");
}


TEST(FlowControlTest, ForStatementWithoutIncrement) {
  const char *source =
      "for (var i = 1; i <= 3;) {\n"
      "  print i;\n"
      "  i = i + 1;\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 1.00\n-> 2.00\n-> 3.00\n");
}

TEST(FlowControlTest, ForStatementWithoutBraces) {
  const char *source =
      "for (var i = 1; i <= 3; i = i + 1)\n"
      "  print i;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 1.00\n-> 2.00\n-> 3.00\n");
}

TEST(FlowControlTest, WhileStatement) {
  const char *source =
      "var count = 3;\n"
      "while (count > 0) {\n"
      "  print count;\n"
      "  count = count - 1;\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 3.00\n-> 2.00\n-> 1.00\n");
}

TEST(FlowControlTest, WhileStatementWithoutBraces) {
  const char *source =
      "var n = 2;\n"
      "while (n < 5)\n"
      "  n = n + 1;\n"
      "print n;\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 5.00\n");
}
