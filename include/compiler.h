#ifndef asas_compiler_h
#define asas_compiler_h

#include "chunk.h"
#include "debug.h"
#include "object.h"
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

class LocalVariable {
public:
  LocalVariable(Token name, int depth)
      : name(name), depth(depth), isCaptured(false) {}
  Token name;
  int depth;
  bool isCaptured;
};

enum FunctionType {
  FUNCTION,
  SCRIPT
};

class Upvalue {
public:
  Upvalue(uint8_t index, bool isLocal)
      : index(index), isLocal(isLocal) {}
  uint8_t index;
  bool isLocal;
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
  Compiler(const char *source, AsasString *fnName, FunctionType type = SCRIPT)
      : scanner_(source), enclosing_(nullptr),
        currentFunction_(new AsasFunction(new Chunk(), fnName)), 
        currentFunctionType_(type)
  {
    Token token(TOKEN_FUNC, "func_main", 0, 0);
    locals_.push_back(LocalVariable(token, 0));
  }

  AsasFunction *compile();

  void declaration();
  void varDeclaration();

  void statement();
  void printStatement();
  void ifStatement();
  void whileStatement();
  void forStatement();
  void expressionStatement();
  void returnStatement();
  void block();
  void declareVariable();
  void functionDeclaration();
  void beginScope();
  void endScope();
  void markInitialized();
  void function(FunctionType type);
  uint8_t argumentsList();

  void expression();
  void call(bool canAssign);
  void grouping(bool canAssign);
  void variable(bool canAssign);
  void string(bool canAssign);
  void number(bool canAssign);
  void unary(bool canAssign);
  void binary(bool canAssign);
  void literal(bool canAssign);
  void defineVariable(uint8_t global);
  void andOperator(bool canAssign);
  void orOperator(bool canAssign);

  int resolveUpvalue(const Token &name);
  int addUpvalue(uint8_t index, bool isLocal);

private:
  Parser parser_;
  Scanner scanner_;
  Compiler *enclosing_;
  AsasFunction* currentFunction_;
  std::vector<Upvalue> upvalues_;
  FunctionType currentFunctionType_;
  std::vector<LocalVariable> locals_;
  int scopeDepth_ = 0;

  void addLocal(const Token &name);
  Chunk *currentChunk() { return currentFunction_->getChunk(); }

  void emitLoop(int loopStart);
  int emitJump(uint8_t instruction);
  void patchJump(int offset);
  void synchronize();
  int resolveLocal(const Token &name);
  static bool identifiersEqual(const Token &a, const Token &b) {
    if (a.length != b.length) return false;
    return std::strncmp(a.start, b.start, a.length) == 0;
  }
  uint8_t parseVariable(const char *errorMessage);
  uint8_t identifierConstant(const Token &name);
  void namedVariable(const Token &name, bool canAssign);

  AsasFunction *endCompiler() { 
    emitReturn();
    AsasFunction* function = currentFunction_;

#ifdef DEBUG_TRACE_EXECUTION
    if (!parser_.hadError) {
      // std::string functionName = currentFunction_->getName().empty()
                                     // ? currentFunction_->getName()
                                     // : "<script>";
      // DebugChunk::disassembleChunk(*currentFunction_->getChunk(), functionName.c_str());
    }
#endif

    return function;
  }
  void emitReturn() { emitByte(OP_NIL); emitByte(OP_RETURN); }
  void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
  }
  uint8_t makeConstant(Value value);
  void emitByte(uint8_t byte) {
    currentChunk()->write(byte, parser_.previous.line);
  }
  void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1); emitByte(byte2);
  }
  void advance();
  void consume(TokenType type, const char *message) {
    if (parser_.current.type == type) return advance();
    errorAtCurrent(message);
  }
  void errorAtCurrent(const char *message) {
    errorAt(parser_.current, message);
  }
  void errorAt(const Token &token, const char *message);
  void error(const char *message) {
    errorAt(parser_.previous, message);
  }

  void parsePrecedence(Precedence precedence);
  bool check(TokenType type) {
    return parser_.current.type == type;
  }
  bool match(TokenType type) {
    if (parser_.current.type != type) return false;
    advance();
    return true;
  }
};

#endif // asas_compiler_h
