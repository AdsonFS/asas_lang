#include "vm.h"
#include "chunk.h"
#include "debug.h"
#include "compiler.h"
#include <algorithm>

InterpretResult VM::interpret(const char *source) {
  Compiler compiler(source, chunk_);
  if (!compiler.compile())
    return INTERPRET_COMPILE_ERROR;
  ip_ = chunk_.getCode().data();
  return run();
}

InterpretResult VM::run() {
  auto readByte = [this]() { return *ip_++; };
  auto readConstant = [this, &readByte]() {
    return chunk_.getConstantAt(readByte());
  };

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    debugVM();
#endif

    uint8_t instruction;
    switch (instruction = readByte()) {
    case OP_CONSTANT: {
      Value constant = readConstant();
      push(constant);
      break;
    }
    case OP_ADD:
      binaryOp([](const Value &a, const Value &b) { return a + b; });
      break;
    case OP_SUBTRACT:
      binaryOp([](const Value &a, const Value &b) { return a - b; });
      break;
    case OP_MULTIPLY:
      binaryOp([](const Value &a, const Value &b) { return a * b; });
      break;
    case OP_DIVIDE:
      binaryOp([](const Value &a, const Value &b) { return a / b; });
      break;
    case OP_NEGATE:
      push(-pop());
      break;
    case OP_RETURN:
      printValue("-> ", pop(), "\n");
      return INTERPRET_OK;
    }
  }
}

void VM::debugVM() {
  printf("\033[1;32m");
  printf("          ");
  // std::stack<Value> tempStack = stack_;
  // std::vector<Value> reverseStack;
  // printf("          y");
  // while (!tempStack.empty()) {
  //   reverseStack.push_back(tempStack.top());
  //   tempStack.pop();
  // }
  // std::reverse(reverseStack.begin(), reverseStack.end());
  // for (const Value &value : reverseStack) {
  //   printf(" ");
  //   printValue(value);
  // }
  printf("\n");
  //
  int offset = static_cast<int>(ip_ - chunk_.getCode().data());
  DebugChunk::disassembleInstruction(chunk_, offset);
}
