#ifndef asas_compiler_h
#define asas_compiler_h

#include "chunk.h"
#include "scanner.h"

enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
};

class Parser {
public:
  Parser() : current(), previous(), hadError(false), panicMode(false) {}

  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
};

class Compiler {
public:
  Compiler(const char *source, Chunk &chunk)
      : chunk_(chunk), compilingChunk_(chunk), scanner_(source) {}
  bool compile();

  void expression();
  void grouping();
  void string();
  void number();
  void unary();
  void binary();
  void literal();

private:
  Chunk &chunk_;
  Chunk &compilingChunk_;
  Scanner scanner_;
  Parser parser_;

  Chunk &currentChunk() { return compilingChunk_; }

  void endCompiler() { emitReturn(); }
  void emitReturn() { emitByte(OP_RETURN); }
  void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
  }
  uint8_t makeConstant(Value value);
  void emitByte(uint8_t byte) { chunk_.write(byte, parser_.previous.line); }
  void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1); emitByte(byte2);
  }
  void advance();
  void consume(TokenType type, const char *message);
  void errorAtCurrent(const char *message) {
    errorAt(parser_.current, message);
  }
  void errorAt(const Token &token, const char *message);
  void error(const char *message) {
    errorAt(parser_.previous, message);
  }

  void parsePrecedence(Precedence precedence);
};

#endif // asas_compiler_h
