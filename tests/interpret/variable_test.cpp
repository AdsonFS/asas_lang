#include <gtest/gtest.h>
#include "../asas_fixture.h"
#include "vm.h"

TEST(VariableTest, SimpleGlobalVariableDeclaration) {
  const char *source =
      "var x = 42;\n"
      "var y = 3.14;\n"
      "print x;\n"
      "print y;\n";

  auto [result, output] = AsasFixture::runSource(source);

  EXPECT_EQ(result, INTERPRET_OK);
  EXPECT_EQ(output, "-> 42.00\n-> 3.14\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}

TEST(VariableTest, GlobalVariableDeclarationAndConcatenation) {
  const char *source =
      "var count = 10;\n"
      "var name = \"asas\" + \"lang\";\n"
      "print name;\n"
      "print count;\n";

  auto [result, output] = AsasFixture::runSource(source);

  EXPECT_EQ(result, INTERPRET_OK);
  EXPECT_EQ(output, "-> asaslang\n-> 10.00\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}

TEST(VariableTest, ConcatenationWithVariables) {
  const char *source =
      "var part1 = \"Hello, \";\n"
      "var part2 = \"World!\";\n"
      "var message = part1 + part2;\n"
      "print message;\n";

  auto [result, output] = AsasFixture::runSource(source);

  EXPECT_EQ(result, INTERPRET_OK);
  EXPECT_EQ(output, "-> Hello, World!\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}

TEST(VariableTest, VariableReassignment) {
  const char *source =
      "var x = 5;\n"
      "print x;\n"
      "x = 10;\n"
      "print x;\n";

  auto [result, output] = AsasFixture::runSource(source);

  EXPECT_EQ(result, INTERPRET_OK);
  EXPECT_EQ(output, "-> 5.00\n-> 10.00\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}

TEST(VariableTest, BooleanVariableDeclarationAndUsage) {
  const char *source =
      "var flag = true;\n"
      "print flag;\n"
      "flag = false;\n"
      "print flag;\n";

  auto [result, output] = AsasFixture::runSource(source);

  EXPECT_EQ(result, INTERPRET_OK);
  EXPECT_EQ(output, "-> true\n-> false\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}

TEST(VariableTest, VariableScopeTest) {
  const char *source =
      "var x = 100;\n"
      "{\n"
      "  var x = 200;\n"
      "  print x;\n"
      "  x = 300;\n"
      "  print x;\n"
      "}\n"
      "print x;\n";

  auto [result, output] = AsasFixture::runSource(source);

  EXPECT_EQ(result, INTERPRET_OK);
  EXPECT_EQ(output, "-> 200.00\n-> 300.00\n-> 100.00\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}

TEST(VariableTest, VariableShadowing) {
  const char *source =
      "var a = 1;\n"
      "print a;\n"
      "{\n"
      "  var a = 2;\n"
      "  print a;\n"
      "  {\n"
      "    var a = 3;\n"
      "    print a;\n"
      "  }\n"
      "  print a;\n"
      "}\n"
      "print a;\n";

  auto [result, output] = AsasFixture::runSource(source);

  EXPECT_EQ(result, INTERPRET_OK);
  EXPECT_EQ(output, "-> 1.00\n-> 2.00\n-> 3.00\n-> 2.00\n-> 1.00\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}

TEST(VariableTest, VariableUsageBeforeDeclaration) {
  const char *source =
      "print x;\n"
      "var x = 50;\n";

  auto [result, output] = AsasFixture::runSourceWithError(source);

  EXPECT_EQ(result, INTERPRET_RUNTIME_ERROR);
  EXPECT_EQ(output, "Undefined variable 'x'.\n[line 1] in script\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}

TEST(VariableTest, ReassignUndefinedVariable) {
  const char *source =
      "y = 20;\n";

  auto [result, output] = AsasFixture::runSourceWithError(source);

  EXPECT_EQ(result, INTERPRET_RUNTIME_ERROR);
  EXPECT_EQ(output, "Undefined variable 'y'.\n[line 1] in script\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}
