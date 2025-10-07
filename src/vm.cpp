#include "vm.h"
#include "chunk.h"
#include "debug.h"

InterpretResult VM::interpret() {
  ip_ = chunk_.getCode().data();

  return run();
}

InterpretResult VM::run() {
  auto readByte = [this]() { return *ip_++; };
  auto readConstant = [this, &readByte]() {
    return chunk_.getConstantAt(readByte());
  };
  auto pop_stack = [&stack = stack_]() {
    Value value = stack.top();
    stack.pop();
    return value;
  };

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    debugVM();
#endif

    uint8_t instruction;
    switch (instruction = readByte()) {
    case OP_CONSTANT: {
      Value constant = readConstant();
      stack_.push(constant);
      break;
    }
    case OP_RETURN: {
      printValue("-> ", pop_stack(), "\n");
      return INTERPRET_OK;
    }
    }
  }
}

void VM::debugVM() {
  printf("\033[1;32m");
  printf("          ");
  std::stack<Value> tempStack = stack_;
  std::stack<Value> reverseStack;
  while (!tempStack.empty()) {
    reverseStack.push(tempStack.top());
    tempStack.pop();
  }
  while (!reverseStack.empty()) {
    printValue("[ ", reverseStack.top(), " ]");
    reverseStack.pop();
  }
  printf("\n");

  int offset = static_cast<int>(ip_ - chunk_.getCode().data());
  DebugChunk::disassembleInstruction(chunk_, offset);
}
