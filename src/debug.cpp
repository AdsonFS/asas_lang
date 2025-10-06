#include "debug.h"
#include "chunk.h"
#include "value.h"

void DebugChunk::disassembleChunk(const Chunk &chunk, const char *name) {
  printf("== %s ==\n", name);

  for (size_t offset = 0; offset < chunk.getCode().size();)
    offset = DebugChunk::disassembleInstruction(chunk, offset);
}
int DebugChunk::disassembleInstruction(const Chunk &chunk, int offset) {
  printf("\033[1;33m");
  offset = DebugChunk::disassembleInstruction_(chunk, offset);
  printf("\033[0m");
  return offset;
}

int DebugChunk::disassembleInstruction_(const Chunk &chunk, int offset) {
  printf("%04d ", offset);
  bool isSameLine =
      (offset > 0 && chunk.getLineAt(offset) == chunk.getLineAt(offset - 1));
  if (isSameLine) printf("   | ");
  else printf("%4d ", chunk.getLineAt(offset));

  uint8_t instruction = chunk.getCode()[offset];
  switch (instruction) {
  case OP_CONSTANT:
    return DebugChunk::constantInstruction("OP_CONSTANT", chunk, offset);
  case OP_RETURN:
    return DebugChunk::simpleInstruction("OP_RETURN", offset);
  default:
    printf("Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}

int DebugChunk::simpleInstruction(const char *name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

int DebugChunk::constantInstruction(const char *name, const Chunk &chunk, int offset) {
  uint8_t constant = chunk.getChunkAt(offset + 1);
  printf("%-16s %4d '", name, constant);
  printValue(chunk.getConstantAt(constant));
  printf("'\n");
  return offset + 2;
}
