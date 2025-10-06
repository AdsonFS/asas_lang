#ifndef asas_chunk_h
#define asas_chunk_h

#include "common.h"

enum OpCode : uint8_t {
  OP_RETURN,
};

class Chunk {
public:
  Chunk() {}
  void write(uint8_t byte) { code.push_back(byte); }
  const std::vector<uint8_t>& getCode() const { return code; }

private:
  std::vector<uint8_t> code;
};


#endif // asas_chunk_h
