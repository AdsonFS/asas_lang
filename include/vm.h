#ifndef asas_vm_h
#define asas_vm_h

#include "chunk.h"
#include <stack>

enum InterpretResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
};

class VM {
public:
  VM() : chunk_(*(new Chunk())), ip_(nullptr) {}
  VM(Chunk &chunk) : chunk_(chunk), ip_(chunk.getCode().data()) {}
  InterpretResult interpret(const char *source);

  ~VM() {
    for (AsasObject* obj : allocatedObjects_)
      delete obj;
  }
private:
  Chunk &chunk_;
  std::stack<Value> stack_;
  const uint8_t *ip_;
  InterpretResult run();

  void push(const Value &value) { stack_.push(value); }
  uint8_t readByte() { return *ip_++; }
  Value pop() {
    Value value = stack_.top();
    stack_.pop();
    return value;
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

  std::vector<AsasObject*> allocatedObjects_;
  template<typename T, typename... Args>
  T* allocateObject(Args&&... args) {
    T* object = new T(std::forward<Args>(args)...);
    allocatedObjects_.push_back(object);
    return object;
  }

};

#endif // asas_vm_h
