#include "vm.h"
#include "chunk.h"
#include "debug.h"
#include "compiler.h"
#include <cstdarg>

InterpretResult VM::interpret(const char *source) {
  AsasString *scriptName = allocateObject<AsasString>("<script>");
  Compiler compiler(source, scriptName, FunctionType::SCRIPT);
  // AsasFunction* function = traceObject(compiler.compile());
  AsasFunction* function = compiler.compile();
  if (function == nullptr)
    return INTERPRET_COMPILE_ERROR;

  AsasClosure* closure = new AsasClosure(function);
  setupGarbageCollector(closure);
  push(closure);
  callFrames_.push_back(CallFrame{closure, 0});
  // DebugChunk::disassembleChunk(*function->getChunk(), "code");

  defineNativeFunctions();
  
  return run();
}

InterpretResult VM::run() {

  for (;;) {
    CallFrame *frame = &callFrames_.back();
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
    case OP_POP_UNTIL: {
      uint8_t slot = frame->getSlot();
      while (stack_.size() > slot) pop();
      break;
    }
    case OP_DEFINE_GLOBAL: {
      const Value& constant = frame->readConstant();
      AsasString* value = ValueHelper::toStringObj(constant);
      // globals_[value->getData()] = pop();
      globals_[value->getData()] = pop();
      break;
    }
    case OP_GET_GLOBAL: {
      const Value& constant = frame->readConstant();
      // const char *variableName = ValueHelper::toStringObj(constant)->getData();
      AsasString* variableName = ValueHelper::toStringObj(constant);
      if (!globals_.contains(variableName->getData())) {
        runtimeError("Undefined variable '%s'.", variableName->getData());
        return INTERPRET_RUNTIME_ERROR;
      }
      push(globals_[variableName->getData()]);
      break;
    }
    case OP_SET_GLOBAL: {
      const Value& constant = frame->readConstant();
      // const char *variableName = ValueHelper::toStringObj(constant)->getData();
      AsasString* variableName = ValueHelper::toStringObj(constant);
      if (!globals_.contains(variableName->getData())) {
        runtimeError("Undefined variable '%s'.", variableName);
        return INTERPRET_RUNTIME_ERROR;
      }
      globals_[variableName->getData()] = peek();
      break;
    }
    case OP_GET_UPVALUE: {
      uint8_t slot = frame->readByte();
      push(*frame->getClosure()->getUpvalueAt(slot)->getLocation());
      break;
    }
    case OP_SET_UPVALUE: {
      uint8_t slot = frame->readByte();
      // *frame->getClosure()->getUpvalueAt(slot)->getLocation() = peek();
      frame->getClosure()->setUpValueAt(slot, peek());
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
    case OP_CALL: {
      int argCount = frame->readByte();
      if (!callValue(peek(argCount), argCount)) {
        runtimeError("Failed to call function.");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_CLOSURE: {
      const Value& constant = frame->readConstant();
      AsasFunction* fn = ValueHelper::toFunctionObj(constant);
      traceObject(fn);
      // traceObject(fn->getAsasStringName());

      AsasClosure* closure = allocateObject<AsasClosure>(fn);
      // TODO -> use allocateObject
      // AsasClosure* closure = new AsasClosure(fn);

      push(closure);

      for (int i = 0; i < fn->getUpvalueCount(); i++) {
        uint8_t isLocal = frame->readByte();
        uint8_t index = frame->readByte();
        if (isLocal) {
          Value* local = &stack_[frame->getSlotAt(index)];
          // Value* local = new Value(stack_[frame->getSlotAt(index)]);
          markSlot(static_cast<int>(frame->getSlotAt(index)));
          closure->addUpvalue(captureUpvalue(local));
        }
        else
          closure->addUpvalue(callFrames_.back().getClosure()->getUpvalueAt(index));
      }

      break;
    }
    case OP_CLOSE_UPVALUE: {
      closeUpValue(&stack_.back());
      pop();
      break;
    }
    case OP_RETURN: 
      Value result = pop();
      if (callFrames_.size() == 1) {
        pop();
        return INTERPRET_OK;
      }
      // for (size_t i = frame->getFunction()->arity + 1; i > 0; i--) pop();
      for (size_t i = stack_.size() - frame->getSlotStartIndex(); i > 0; i--) pop();
      callFrames_.pop_back();
      push(result);
      break;
    }
  }
}

void VM::closeUpValue(Value *value) {
  if (!openUpvalues_.contains(value)) return;

  AsasUpvalue* upvalue = openUpvalues_[value];
  upvalue->close();
  openUpvalues_.erase(value);
}

AsasUpvalue* VM::captureUpvalue(Value* local) {
  if (openUpvalues_.contains(local))
    return openUpvalues_[local];
  // return openUpvalues_[local] = new AsasUpvalue(local);
  return openUpvalues_[local] = allocateObject<AsasUpvalue>(local);
}

bool VM::callValue(const Value &callee, int argCount) {
  auto object = std::get_if<AsasObject*>(&callee);
  if (object == nullptr) {
    runtimeError("Can only call functions and classes.");
    return false;
  }

  auto closureValue = dynamic_cast<AsasClosure*>(*object);
  if (closureValue != nullptr) return handleClosureCall(closureValue, argCount);

  auto nativeFunctionValue = dynamic_cast<AsasNativeFunction*>(*object);
  if (nativeFunctionValue != nullptr) return handleNativeFunctionCall(nativeFunctionValue, argCount);

  runtimeError("Can only call functions and classes.");
  return false;
}

bool VM::handleClosureCall(AsasClosure* closure, int argCount) {
  AsasFunction* functionValue = closure->getFunction();
  if (functionValue == nullptr) {
    runtimeError("Can only call functions and classes.");
    return false;
  }

  if (argCount != functionValue->arity) {
    runtimeError("Expected %d arguments but got %d.", functionValue->arity, argCount);
    return false;
  }

  if (callFrames_.size() >= 256) { runtimeError("Stack overflow."); return false; }
  callFrames_.push_back(CallFrame{
    closure,
    (stack_.size()) - argCount - 1 // -1 for the function itself
  });
  return true;
}

bool VM::handleNativeFunctionCall(AsasNativeFunction* nativeFn, int argCount) {
  std::vector<Value> args;
  for (int i = 0; i < argCount; i++) {
    args.push_back(peek(argCount - 1 - i));
  }
  pop();
  push(nativeFn->call(args));
  return true;
}

void VM::debugVM() {
  printf("\n\033[1;32m");
  printf("          STACK:");
  for (const Value &value : stack_)
    printValue(" [ ", value, " ]");
  printf("\n");

  callFrames_.back().debugCF(callFrames_.size() - 1);
}

Value VM::runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  AsasFunction *upFunction = nullptr;
  for (int i = static_cast<int>(callFrames_.size()) - 1; i >= 0; i--) {
    CallFrame &frame = callFrames_[i];
    AsasFunction *function = frame.getFunction();

    if (upFunction == function) continue;
    upFunction = function;

    int line = frame.getCurrentLine();
    fprintf(stderr, "[line %d] in ", line);

    if (function->getName().empty()) fprintf(stderr, "script\n");
    else fprintf(stderr, "%s()\n", function->getName().c_str());
  }

  stack_ = std::vector<Value>();
  return std::monostate();
}

void VM::defineNativeFunctions() {
  // globals_["clock"] = new AsasNativeFunction(
  //   [](const std::vector<Value>& args) -> Value {
  //     return static_cast<double>(clock()) / CLOCKS_PER_SEC;
  //   },
  //   "clock"
  // );
  //
  // globals_["delay"] = new AsasNativeFunction(
  //   [](const std::vector<Value>& args) -> Value {
  //     if (args.size() != 1 || !std::holds_alternative<double>(args[0])) {
  //       return std::monostate{};
  //     }
  //     double seconds = std::get<double>(args[0]);
  //     clock_t start_time = clock();
  //     while (static_cast<double>(clock() - start_time) / CLOCKS_PER_SEC < seconds) {
  //       // Busy-wait loop
  //     }
  //     return std::monostate{};
  //   },
  //   "delay"
  // );
}
