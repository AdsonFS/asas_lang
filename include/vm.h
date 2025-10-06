#ifndef asas_vm_h
#define asas_vm_h

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
  const uint8_t *ip_;
  InterpretResult run();
};

#endif // asas_vm_h
