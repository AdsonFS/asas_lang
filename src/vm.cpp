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
    case OP_GET_GLOBAL: {
      const Value& constant = readConstant();
      AsasString* name = ValueHelper::convertToStringObj(constant);
      auto it = globals_.find(name->getData());
      if (it == globals_.end()) {
        runtimeError("Undefined variable '%s'.", name->getData());
        return INTERPRET_RUNTIME_ERROR;
      }
      push(it->second);
      break;
    }
    case OP_DEFINE_GLOBAL: {
      const Value& constant = readConstant();
      AsasString* value = ValueHelper::convertToStringObj(constant);
      globals_[value->getData()] = pop();
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

Value VM::runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = static_cast<size_t>(ip_ - chunk_.getCode().data()) - 1;
  int line = chunk_.getLineAt(instruction);
  fprintf(stderr, "[line %d] in script\n", line);
  stack_ = std::stack<Value>();
  return std::monostate();
}
