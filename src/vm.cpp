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

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    int offset = static_cast<int>(ip_ - chunk_.getCode().data());
    DebugChunk::disassembleInstruction(chunk_, offset);
#endif

    uint8_t instruction;
    switch (instruction = readByte()) {
    case OP_CONSTANT: {
      Value constant = readConstant();
      printValue(constant);
      printf("\n");
      break;
    }
    case OP_RETURN: {
      return INTERPRET_OK;
    }
    }
  }
}
