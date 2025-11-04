#ifndef asas_asas_fixture_h
#define asas_asas_fixture_h

#include <gtest/gtest.h>
#include <utility>
#include "vm.h"

class AsasFixture {
public:
  static std::pair<InterpretResult, std::string> runSourceWithSuccess(const char *source) {
    VM* vm = new VM();
    testing::internal::CaptureStdout();
    InterpretResult result = vm->interpret(source);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(vm->stackSize(), 0);
    delete vm;

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AsasString::getRefCountObjects(), 0);
    EXPECT_EQ(AsasFunction::getRefCountObjects(), 0);
    // EXPECT_EQ(AsasUpvalue::getRefCountObjects(), 0);
    EXPECT_EQ(AsasClosure::getRefCountObjects(), 0);

    return {result, output};
  }

  static std::pair<InterpretResult, std::string> runSourceWithError(const char *source) {
    VM* vm = new VM();
    testing::internal::CaptureStderr();
    InterpretResult result = vm->interpret(source);
    std::string output = testing::internal::GetCapturedStderr();

    EXPECT_EQ(vm->stackSize(), 0);
    delete vm;

    EXPECT_EQ(result, INTERPRET_RUNTIME_ERROR);
    EXPECT_EQ(AsasString::getRefCountObjects(), 0);
    EXPECT_EQ(AsasFunction::getRefCountObjects(), 0);

    return {result, output};
  }
};

#endif // asas_asas_fixture_h
