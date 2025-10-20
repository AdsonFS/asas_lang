#ifndef asas_asas_fixture_h
#define asas_asas_fixture_h

#include <gtest/gtest.h>
#include <utility>
#include "vm.h"

class AsasFixture {
public:
  static std::pair<InterpretResult, std::string> runSource(const char *source) {
    VM* vm = new VM();
    testing::internal::CaptureStdout();
    InterpretResult result = vm->interpret(source);
    std::string output = testing::internal::GetCapturedStdout();
    delete vm;

    return {result, output};
  }

  static std::pair<InterpretResult, std::string> runSourceWithError(const char *source) {
    VM* vm = new VM();
    testing::internal::CaptureStderr();
    InterpretResult result = vm->interpret(source);
    std::string output = testing::internal::GetCapturedStderr();
    delete vm;

    return {result, output};
  }
};

#endif // asas_asas_fixture_h
