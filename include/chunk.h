#ifndef asas_chunk_h
#define asas_chunk_h

#include "value.h"

enum OpCode : uint8_t {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_DEFINE_GLOBAL,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP,
  OP_CALL,
  OP_RETURN,
};

class Chunk {
public:
  void write(const uint8_t &byte, int line) {
    code_.push_back(byte);
    lines_.push_back(line);
  }
  int addConstant(const Value &value) {
    constants_.write(value);
    return constants_.size() - 1;
  }
  const Value &getConstantAt(size_t index) const { return constants_.getAt(index); }
  const uint8_t &getChunkAt(size_t index) const { return code_[index]; }
  const int &getLineAt(size_t index) const { return lines_[index]; }
  const std::vector<uint8_t> &getCode() const { return code_; }
  void setAt(size_t index, uint8_t byte) { code_[index] = byte; }

  // ~Chunk() = default;
  ~Chunk() {
    printf("Destroying Chunk with %zu constants.\n", constants_.size());
  }

private:
  std::vector<uint8_t> code_;
  std::vector<int> lines_;
  DataValue constants_;
};

#endif // asas_chunk_h
