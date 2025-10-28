#include "vm.h"
#include "chunk.h"
#include "debug.h"
#include "compiler.h"
#include <cstdarg>

InterpretResult VM::interpret(const char *source) {
  Compiler compiler(source);
  AsasFunction* function = compiler.compile();
  if (function == nullptr)
    return INTERPRET_COMPILE_ERROR;

  push(function);
  callFrames_.push_back(CallFrame{
    function,
    stack_.size()
  });
      
  // chunk_ = *function->getChunk();
  // ip_ = chunk_.getCode().data();
  return run();
}

InterpretResult VM::run() {
  CallFrame *frame = &callFrames_.back();

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    debugVM();
#endif
    uint8_t instruction;
    switch (instruction = frame->readByte()) {
    case OP_CONSTANT: push(frame->readConstant()); break;
    case OP_NIL: push(std::monostate{}); break;
    case OP_TRUE: push(true); break;
    case OP_FALSE: push(false); break;
    case OP_POP: pop(); break;
    case OP_DEFINE_GLOBAL: {
      const Value& constant = frame->readConstant();
      AsasString* value = ValueHelper::toStringObj(constant);
      globals_[value->getData()] = pop();
      break;
    }
    case OP_GET_GLOBAL: {
      const Value& constant = frame->readConstant();
      const char *variableName = ValueHelper::toStringObj(constant)->getData();
      if (!globals_.contains(variableName)) {
        runtimeError("Undefined variable '%s'.", variableName);
        return INTERPRET_RUNTIME_ERROR;
      }
      push(globals_[variableName]);
      break;
    }
    case OP_SET_GLOBAL: {
      const Value& constant = frame->readConstant();
      const char *variableName = ValueHelper::toStringObj(constant)->getData();
      if (!globals_.contains(variableName)) {
        runtimeError("Undefined variable '%s'.", variableName);
        return INTERPRET_RUNTIME_ERROR;
      }
      globals_[variableName] = peek();
      break;
    }
    case OP_GET_LOCAL: {
      uint8_t slot = frame->getSlot();
      push(stack_[slot]);
      break;
    }
    case OP_SET_LOCAL: {
      uint8_t slot = frame->getSlot();
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
    case OP_JUMP: {
      uint16_t offset = frame->readShort();
      frame->incrementIP(offset);
      break;
    }
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = frame->readShort();
      if (!ValueHelper::toBool(peek()))
        frame->incrementIP(offset);
      break;
    }
    case OP_LOOP: {
      uint16_t offset = frame->readShort();
      frame->incrementIP(-offset);
      break;
    }
    case OP_RETURN: return INTERPRET_OK;
    }
  }
}

void VM::debugVM() {
  printf("\n\033[1;32m");
  printf("          STACK:");
  for (const Value &value : stack_)
    printValue(" [ ", value, " ]");
  printf("\n");

  callFrames_.back().debugCF();
  // int offset = static_cast<int>(ip_ - chunk_.getCode().data());
  // DebugChunk::disassembleInstruction(chunk_, offset);
}

Value VM::runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  int line = callFrames_.back().getCurrentLine();
  fprintf(stderr, "[line %d] in script\n", line);
  stack_ = std::vector<Value>();
  return std::monostate();
}
