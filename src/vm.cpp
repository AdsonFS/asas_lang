#include "vm.h"
#include "chunk.h"
#include "debug.h"
#include "compiler.h"

InterpretResult VM::interpret() {
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
