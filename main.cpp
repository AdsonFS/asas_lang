#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main() {
  Chunk chunk;
  VM vm(chunk);

  int constantIndex = chunk.addConstant(1.2);
  chunk.write(OP_CONSTANT, 123);
  chunk.write(constantIndex, 123);

  int constantIndex2 = chunk.addConstant(3.4);
  chunk.write(OP_CONSTANT, 123);
  chunk.write(constantIndex2, 123);


  chunk.write(OP_RETURN, 125);

  vm.interpret();
  DebugChunk::disassembleChunk(chunk, "test chunk");

  return 0;
}
