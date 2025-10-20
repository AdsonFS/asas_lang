#ifndef asas_debug_h
#define asas_debug_h

#include <stdio.h>
#include "chunk.h"

class DebugChunk {
public:
  static void disassembleChunk(const Chunk &chunk, const char *name);
  static int disassembleInstruction(const Chunk &chunk, int offset);

private:
  static int disassembleInstruction_(const Chunk &chunk, int offset);
  static int simpleInstruction(const char *name, int offset);
  static int constantInstruction(const char *name, const Chunk &chunk, int offset);
  static int byteInstruction(const char *name, const Chunk &chunk, int offset);
};

#endif // asas_debug_h
