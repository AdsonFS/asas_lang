#include "vm.h"
#include <gtest/gtest.h>

TEST(NumericExpressionTest, SimpleExpressions) {
  char buffer[50];
  const char *expressions[] = {
    "print 2 + 3 * 4 - 5 / (1 + 1);",  // 2 + 12 - 2.5 = 11
    "print 10 - 2 * 3 + 4 / 2;",       // 10 - 6 + 2 = 6
    "print (1 + 2) * (3 + 4);",        // 3 * 7 = 21
    "print 5 + 3 * (10 - 4) / 2;",     // 5 + 3 * 6 / 2 = 5 + 9 = 14
    "print 3 + 4 * 2 / (1 - 5) + 6;",  // 3 + 8 / -4 + 6 = 3 - 2 + 6 = 7
  };
  double answers[] = {11.5, 6.0, 21.0, 14.0, 7.0};
  for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); ++i) {
    VM vm;
    testing::internal::CaptureStdout();
    InterpretResult result = vm.interpret(expressions[i]);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(result, INTERPRET_OK);
    snprintf(buffer, sizeof(buffer), "-> %.2f\n", answers[i]);
    EXPECT_EQ(output, std::string(buffer));
  }
}
