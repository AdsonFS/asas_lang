#ifndef asas_vm_h
#define asas_vm_h

#include "chunk.h"
#include "debug.h"
#include "object.h"
#include <stack>
#include <unordered_map>

enum InterpretResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
};

class CallFrame {
public:
  // CallFrame(AsasFunction *function, size_t slotStartIndex)
  //     : ip_(function->getChunk()->getCode().data()),
  //       function_(function), slotStartIndex_(slotStartIndex) { }
  CallFrame(AsasClosure *closure, size_t slotStartIndex)
      : ip_(closure->getFunction()->getChunk()->getCode().data()),
        closure_(closure),
        function_(closure->getFunction()),
        slotStartIndex_(static_cast<int>(slotStartIndex)) { }

  const uint8_t readByte() { return *ip_++; }
  const uint16_t readShort() { return (uint16_t)((readByte() << 8) | readByte()); }
  const Value readConstant() { return function_->getChunk()->getConstantAt(readByte()); }
  const void incrementIP(int offset) { ip_ += offset; }
  const uint8_t getSlot() { return readByte() + slotStartIndex_ + 0; }
  const void debugCF() {
    size_t offset = static_cast<size_t>(ip_ - function_->getChunk()->getCode().data());
    DebugChunk::disassembleInstruction(*function_->getChunk(), offset);
  }
  const int getCurrentLine() {
    size_t offset = static_cast<size_t>(ip_ - function_->getChunk()->getCode().data()) - 1;
    return function_->getChunk()->getLineAt(offset);
  }
  AsasFunction* getFunction() { return function_; }

private:
  const uint8_t *ip_;
  AsasClosure *closure_;
  AsasFunction *function_;
  const int slotStartIndex_;
};

class VM {
public:
  // VM() : ip_(nullptr) {}
  // VM(Chunk chunk) : chunk_(chunk), ip_(chunk_.getCode().data()) {}
  InterpretResult interpret(const char *source);
  int stackSize() const { return stack_.size(); }

  ~VM() {
    for (AsasObject *obj : allocatedObjects_)
      delete obj;
  }
private:
  // Chunk chunk_;
  // const uint8_t *ip_;
  std::vector<Value> stack_;
  std::vector<CallFrame> callFrames_;
  std::unordered_map<std::string, Value> globals_;

  InterpretResult run();

  void defineNativeFunctions();
  void push(const Value &value) { stack_.push_back(value); }
  Value pop() {
    Value value = stack_.back();
    stack_.pop_back();
    return value;
  }
  Value peek(int distance = 0) {
    return stack_[stack_.size() - 1 - distance];
  }

  Value runtimeError(const char *format, ...);
  void opEqual();
  void opGreater();
  void opLess();
  void opAdd();
  void opSubtract();
  void opMultiply();
  void opDivide();
  void opNegate();
  void opNot();

  void debugVM();
  bool callValue(const Value &callee, int argCount);
  bool handleNativeFunctionCall(AsasNativeFunction* nativeFn, int argCount);
  bool handleClosureCall(AsasClosure* closure, int argCount);

  std::vector<AsasObject*> allocatedObjects_;
  template<typename T, typename... Args>
  T* allocateObject(Args&&... args) {
    T* object = new T(std::forward<Args>(args)...);
    allocatedObjects_.push_back(object);
    return object;
  }
};

#endif // asas_vm_h
