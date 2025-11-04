#ifndef asas_vm_h
#define asas_vm_h

#include "chunk.h"
#include "debug.h"
#include "object.h"
#include <bitset>
#include <stack>
#include <unordered_map>

#define STACK_MAX 256

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
  const uint8_t getSlot() { return readByte() + slotStartIndex_; }
  uint8_t getSlotAt(int index) { return slotStartIndex_ + index; }
  int getSlotStartIndex() const { return slotStartIndex_; }
  const void debugCF(int frameIndex) {
    size_t offset = static_cast<size_t>(ip_ - function_->getChunk()->getCode().data());
    DebugChunk::disassembleInstruction(*function_->getChunk(), offset, frameIndex);
  }
  const int getCurrentLine() {
    size_t offset = static_cast<size_t>(ip_ - function_->getChunk()->getCode().data()) - 1;
    return function_->getChunk()->getLineAt(offset);
  }
  AsasFunction* getFunction() const { return function_; }
  AsasClosure* getClosure() const { return closure_; }

private:
  const uint8_t *ip_;
  AsasClosure *closure_;
  AsasFunction *function_;
  const int slotStartIndex_;
};
class VMStack {
public:
  VMStack() {
    stack_.reserve(static_cast<size_t>(STACK_MAX));
  }
  size_t size() const { return static_cast<int>(stack_.size()); }
  void push(const Value &value) { 
    // [TODO] -> fix runtimeError
    // if (stack_.size() >= STACK_MAX) 
      // return void(runtimeError("Stack overflow."));
    stack_.emplace_back(value, idSlot_++);
  }
  std::pair<Value, int>& at(int index) {
    return stack_[index];
  }
  Value getAt(int index) const {
    return stack_[index].first;
  }
  void setAt(int index, const Value &value) {
    stack_[index].first = value;
  }
  Value pop() {
    Value value = stack_.back().first;
    stack_.pop_back();
    return value;
  }
  Value peek(int distance = 0) {
    return stack_[stack_.size() - 1 - distance].first;
  }
  Value &topRef() {
    return stack_.back().first;
  }
  void clear() { stack_.clear(); }

private:
  std::vector<std::pair<Value, int>> stack_;
  inline static int idSlot_ = 0;
};

class VM {
public:
  VM() { 
    stack_.reserve(static_cast<size_t>(STACK_MAX));
  }
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
  std::bitset<STACK_MAX> markedFlags_;
  // std::vector<bool> markedFlags_;

  std::vector<CallFrame> callFrames_;
  std::unordered_map<std::string, Value> globals_;
  std::unordered_map<Value*, AsasUpvalue*> openUpvalues_;

  InterpretResult run();

  void closeUpValue(Value* value);
  void defineNativeFunctions();
  AsasUpvalue* captureUpvalue(Value* local);

  void markSlot(int index) { markedFlags_.set(index); }
  void unmarkSlot(int index) { markedFlags_.reset(index); }

  void push(const Value &value) { 
    if (stack_.size() >= STACK_MAX) 
      return void(runtimeError("Stack overflow."));
    stack_.push_back(value);
  }
  Value pop() {
    int index = static_cast<int>(stack_.size()) - 1;
    if (markedFlags_.test(index)) {
      closeUpValue(&stack_.back());
      unmarkSlot(index);
    }
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

  // garbage collection
  std::vector<AsasObject*> allocatedObjects_;
  template<typename T, typename... Args>
  T* allocateObject(Args&&... args) {
    T* object = new T(std::forward<Args>(args)...);
    allocatedObjects_.push_back(object);
    return object;
  }
  template<typename T>
  T* mapObject(T* object) {
    allocatedObjects_.push_back(object);
    return object;
  }
  void collectGarbage();

};

#endif // asas_vm_h
