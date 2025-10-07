#ifndef asas_vm_h
#define asas_vm_h

#include <stack>
#include "chunk.h"

enum InterpretResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
};

class VM {
public:
  VM(Chunk &chunk) : chunk_(chunk), ip_(chunk.getCode().data()) {}
  const uint8_t readByte() { return *ip_++; }
  InterpretResult interpret();

private:
  Chunk &chunk_;
  std::stack<Value> stack_;
  const uint8_t *ip_;
  InterpretResult run();
  void debugVM();
};

#endif // asas_vm_h
