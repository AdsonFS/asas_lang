#include "vm.h"
#include "chunk.h"
#include "debug.h"
#include "compiler.h"
#include <cstdarg>

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
    // debugVM();
#endif
    uint8_t instruction;
    switch (instruction = readByte()) {
    case OP_CONSTANT: push(readConstant()); break;
    case OP_NIL: push(std::monostate{}); break;
    case OP_TRUE: push(true); break;
    case OP_FALSE: push(false); break;
    case OP_POP: pop(); break;
    case OP_DEFINE_GLOBAL: {
      const Value& constant = readConstant();
      AsasString* value = ValueHelper::convertToStringObj(constant);
      globals_[value->getData()] = pop();
      break;
    }
    case OP_GET_GLOBAL: {
      const Value& constant = readConstant();
      const char *variableName = ValueHelper::convertToStringObj(constant)->getData();
      if (!globals_.contains(variableName)) {
        runtimeError("Undefined variable '%s'.", variableName);
        return INTERPRET_RUNTIME_ERROR;
      }
      push(globals_[variableName]);
      break;
    }
    case OP_SET_GLOBAL: {
      const Value& constant = readConstant();
      const char *variableName = ValueHelper::convertToStringObj(constant)->getData();
      if (!globals_.contains(variableName)) {
        runtimeError("Undefined variable '%s'.", variableName);
        return INTERPRET_RUNTIME_ERROR;
      }
      globals_[variableName] = peek();
      break;
    }
    case OP_GET_LOCAL: {
      uint8_t slot = readByte();
      push(stack_[slot]);
      break;
    }
    case OP_SET_LOCAL: {
      uint8_t slot = readByte();
      stack_[slot] = peek();
      break;
    }
    case OP_EQUAL: opEqual(); break;
    case OP_GREATER: opGreater(); break;
    case OP_LESS: opLess(); break;
    case OP_ADD: opAdd(); break;
    case OP_SUBTRACT: opSubtract(); break;
    case OP_MULTIPLY: opMultiply(); break;
    case OP_DIVIDE: opDivide(); break;
    case OP_NOT: opNot(); break;
    case OP_NEGATE: opNegate(); break;
    case OP_PRINT: printValue("-> ", pop(), "\n"); break;
    case OP_RETURN: return INTERPRET_OK;
    }
  }
}

void VM::debugVM() {
  printf("\033[1;32m");
  printf("          STACK:");
  for (const Value &value : stack_)
    printValue(" [ ", value, " ]");
  printf("\n");

  int offset = static_cast<int>(ip_ - chunk_.getCode().data());
  DebugChunk::disassembleInstruction(chunk_, offset);
}

Value VM::runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = static_cast<size_t>(ip_ - chunk_.getCode().data()) - 1;
  int line = chunk_.getLineAt(instruction);
  fprintf(stderr, "[line %d] in script\n", line);
  stack_ = std::vector<Value>();
  return std::monostate();
}
