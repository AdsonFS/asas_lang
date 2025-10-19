#include "vm.h"
#include <gtest/gtest.h>

TEST(VariableTest, SimpleVariableDeclaration) {
  const char *source =
      "var x = 42;\n"
      "var y = 3.14;\n"
      "print x;\n"
      "print y;\n";

  VM* vm = new VM();
  testing::internal::CaptureStdout();
  InterpretResult result = vm->interpret(source);
  std::string output = testing::internal::GetCapturedStdout();
  delete vm;

  EXPECT_EQ(result, INTERPRET_OK);
  EXPECT_EQ(output, "-> 42.00\n-> 3.14\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}

TEST(VariableTest, VariableDeclarationAndConcatenation) {
  const char *source =
      "var count = 10;\n"
      "var name = \"asas\" + \"lang\";\n"
      "print name;\n"
      "print count;\n";

  VM* vm = new VM();
  testing::internal::CaptureStdout();
  InterpretResult result = vm->interpret(source);
  std::string output = testing::internal::GetCapturedStdout();
  delete vm;

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

  VM* vm = new VM();
  testing::internal::CaptureStdout();
  InterpretResult result = vm->interpret(source);
  std::string output = testing::internal::GetCapturedStdout();
  delete vm;

  EXPECT_EQ(result, INTERPRET_OK);
  EXPECT_EQ(output, "-> Hello, World!\n");
  EXPECT_EQ(AsasString::getRefCountObjects(), 0);
}
