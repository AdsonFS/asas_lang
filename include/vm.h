#ifndef asas_vm_h
#define asas_vm_h

#include "chunk.h"
#include "debug.h"
#include "object.h"
#include <bitset>
#include <set>
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
  std::set<AsasObject*> allocatedObjects_;

  template<typename T, typename... Args>
  T* allocateObject(Args&&... args) {
    T* object = new T(std::forward<Args>(args)...);
    allocatedObjects_.insert(object);

  #ifdef DEBUG_LOG_GC
    printf("Allocated object %p of type %s, size %zu\n", 
           (void*)object, typeid(T).name(), sizeof(T));
  #endif

    // push object to stack to avoid being collected immediately
    // push(reinterpret_cast<Value>(object));
    // AsasObject* objPtr = reinterpret_cast<AsasObject*>(object);
    push(reinterpret_cast<AsasObject*>(object));
    // TODO: trigger garbage collection based on some threshold
    collectGarbage();


    pop();

    return object;
  }
  template<typename T>
  T* traceObject(T* object) {
    allocatedObjects_.insert(object);
#ifdef DEBUG_LOG_GC
    printf("Traced object %p of type %s, size %zu\n", 
           (void*)object, typeid(T).name(), sizeof(T));
#endif

    push(reinterpret_cast<AsasObject*>(object));
    collectGarbage();
    pop();
    return object;
  }
  void collectGarbage();
  void markRoots();
  void markValue(Value *value);
  void freeObjects();
  int instructionCount_ = 0;

};

#endif // asas_vm_h
