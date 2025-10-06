#include "chunk.h"
#include "debug.h"

int main() {
  Chunk chunk;
  chunk.write(OP_RETURN);

  DebugChunk::disassembleChunk(chunk, "test chunk");

  return 0;
}
