#include "debug.h"
#include "chunk.h"
#include "value.h"
#include "object.h"

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
  if (isSameLine)
    printf("   | ");
  else
    printf("%4d ", chunk.getLineAt(offset));

  uint8_t instruction = chunk.getCode()[offset];
  switch (instruction) {
  case OP_CONSTANT: return DebugChunk::constantInstruction("OP_CONSTANT", chunk, offset);
  case OP_NIL: return DebugChunk::simpleInstruction("OP_NIL", offset);
  case OP_TRUE: return DebugChunk::simpleInstruction("OP_TRUE", offset);
  case OP_FALSE: return DebugChunk::simpleInstruction("OP_FALSE", offset);
  case OP_POP: return DebugChunk::simpleInstruction("OP_POP", offset);
  case OP_DEFINE_GLOBAL: return DebugChunk::constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
  case OP_GET_GLOBAL: return DebugChunk::constantInstruction("OP_GET_GLOBAL", chunk, offset);
  case OP_SET_GLOBAL: return DebugChunk::constantInstruction("OP_SET_GLOBAL", chunk, offset);
  case OP_GET_UPVALUE: return DebugChunk::byteInstruction("OP_GET_UPVALUE", chunk, offset);
  case OP_SET_UPVALUE: return DebugChunk::byteInstruction("OP_SET_UPVALUE", chunk, offset);
  case OP_GET_LOCAL: return DebugChunk::byteInstruction("OP_GET_LOCAL", chunk, offset);
  case OP_SET_LOCAL: return DebugChunk::byteInstruction("OP_SET_LOCAL", chunk, offset);
  case OP_EQUAL: return DebugChunk::simpleInstruction("OP_EQUAL", offset);
  case OP_GREATER: return DebugChunk::simpleInstruction("OP_GREATER", offset);
  case OP_LESS: return DebugChunk::simpleInstruction("OP_LESS", offset);
  case OP_ADD: return DebugChunk::simpleInstruction("OP_ADD", offset);
  case OP_SUBTRACT: return DebugChunk::simpleInstruction("OP_SUBTRACT", offset);
  case OP_MULTIPLY: return DebugChunk::simpleInstruction("OP_MULTIPLY", offset);
  case OP_DIVIDE: return DebugChunk::simpleInstruction("OP_DIVIDE", offset);
  case OP_NOT: return DebugChunk::simpleInstruction("OP_NOT", offset);
  case OP_NEGATE: return DebugChunk::simpleInstruction("OP_NEGATE", offset);
  case OP_PRINT: return DebugChunk::simpleInstruction("OP_PRINT", offset);
  case OP_JUMP: return DebugChunk::jumpInstruction("OP_JUMP", 1, chunk, offset);
  case OP_JUMP_IF_FALSE: return DebugChunk::jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
  case OP_LOOP: return DebugChunk::jumpInstruction("OP_LOOP", -1, chunk, offset);
  case OP_CALL: return DebugChunk::byteInstruction("OP_CALL", chunk, offset);
  case OP_CLOSURE: {
    uint8_t constant = chunk.getChunkAt(offset + 1);
    printf("%-16s %4d '", "OP_CLOSURE", constant);
    printValue(chunk.getConstantAt(constant));
    printf("'\n");

    AsasFunction* function = ValueHelper::toFunctionObj(chunk.getConstantAt(constant));
    for (int j = 0; j < function->getUpvalueCount(); j++) {
      uint8_t isLocal = chunk.getChunkAt(offset + 2 + j * 2);
      uint8_t index = chunk.getChunkAt(offset + 3 + j * 2);
      printf("%04d      |                     %s %d\n",
             offset + 2 + j * 2,
             isLocal ? "local" : "upvalue", index);
    }

    return offset + 2 + function->getUpvalueCount() * 2;
  }
  case OP_CLOSE_UPVALUE: return DebugChunk::simpleInstruction("OP_CLOSE_UPVALUE", offset);
  case OP_RETURN: return DebugChunk::simpleInstruction("OP_RETURN", offset);
  default:
    printf("Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}

int DebugChunk::simpleInstruction(const char *name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

int DebugChunk::constantInstruction(const char *name, const Chunk &chunk,
                                    int offset) {
  uint8_t constant = chunk.getChunkAt(offset + 1);
  printf("%-16s %4d '", name, constant);
  printValue(chunk.getConstantAt(constant));
  printf("'\n");
  return offset + 2;
}

int DebugChunk::byteInstruction(const char *name, const Chunk &chunk, int offset) {
  uint8_t slot = chunk.getChunkAt(offset + 1);
  printf("%-16s %4d\n", name, slot);
  return offset + 2;
}

int DebugChunk::jumpInstruction(const char *name, int sign, const Chunk &chunk, int offset) {
  uint16_t jump = (uint16_t)(chunk.getChunkAt(offset + 1) << 8);
  jump |= chunk.getChunkAt(offset + 2);
  printf("%-16s %4d -> %d\n", name, offset,
         offset + 3 + sign * jump);
  return offset + 3;
}
