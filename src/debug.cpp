#include "debug.h"

void DebugChunk::disassembleChunk(const Chunk& chunk, const char* name) {
  printf("== %s ==\n", name);

  for (size_t offset = 0; offset < chunk.getCode().size();)
    offset = disassembleInstruction(chunk, offset);
}

int DebugChunk::disassembleInstruction(const Chunk& chunk, int offset) {
  printf("%04d ", offset);

  uint8_t instruction = chunk.getCode()[offset];
  switch (instruction) {
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
      return offset + 1;
    default:
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
  }
}

int DebugChunk::simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}
