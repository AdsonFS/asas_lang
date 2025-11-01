#include <cstdint>
#include <cstdlib>
#include "chunk.h"
#include "stdio.h"

#include "parse_rule.h"
#include "compiler.h"
#include "scanner.h"
#include "object.h"
#include "common.h"


AsasFunction *Compiler::compile() {
  advance();
  while (!match(TOKEN_EOF)) {
    declaration();
  }
  return parser_.hadError ? nullptr : endCompiler();
}

void Compiler::declaration() {
  if (match(TOKEN_VAR)) varDeclaration();
  else if (match(TOKEN_FUNC)) functionDeclaration();
  else statement();

  if (parser_.panicMode) synchronize();
}

void Compiler::functionDeclaration() {
  uint8_t global = parseVariable("Expect function name.");
  markInitialized();
  function(FunctionType::FUNCTION);
  defineVariable(global);
}

void Compiler::function(FunctionType type) {
  std::string functionName(parser_.previous.start, parser_.previous.length);
  Compiler functionCompiler(scanner_.getRemainingSource(), type, functionName);
  functionCompiler.parser_ = parser_;
  functionCompiler.enclosing_ = this;
  
  functionCompiler.beginScope();
  functionCompiler.consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

  if (!functionCompiler.check(TOKEN_RIGHT_PAREN)) {
    do {
      functionCompiler.currentFunction_->arity++;
      if (functionCompiler.currentFunction_->arity > 255)
        functionCompiler.errorAtCurrent("Can't have more than 255 parameters.");

      uint8_t paramConstant = functionCompiler.parseVariable("Expect parameter name.");
      functionCompiler.defineVariable(paramConstant);
    } while (functionCompiler.match(TOKEN_COMMA));
  }


  functionCompiler.consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
  functionCompiler.consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
  functionCompiler.block();
  
  AsasFunction* function = functionCompiler.endCompiler();
  emitBytes(OP_CLOSURE, makeConstant(function));

  for (auto &upvalue : functionCompiler.upvalues_) {
    emitByte(upvalue.isLocal);
    emitByte(upvalue.index);
  }

  parser_ = functionCompiler.parser_;
  scanner_ = functionCompiler.scanner_;
}

void Compiler::varDeclaration() {
  uint8_t global = parseVariable("Expect variable name.");

  if (match(TOKEN_EQUAL)) expression();
  else emitByte(OP_NIL);

  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
  defineVariable(global);
}

void Compiler::ifStatement() {
  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int thenJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();

  int elseJump = emitJump(OP_JUMP);

  patchJump(thenJump);
  emitByte(OP_POP);

  if (match(TOKEN_ELSE)) statement();
  patchJump(elseJump);
}

int Compiler::emitJump(uint8_t instruction) {
  emitByte(instruction);
  emitByte(0xff);
  emitByte(0xff);
  return currentChunk()->getCode().size() - 2;
}

void Compiler::patchJump(int offset) {
  // -2 to adjust for the bytecode for the jump offset itself.
  int jump = currentChunk()->getCode().size() - offset - 2;

  if (jump > 0xffff) error("Too much code to jump over.");

  currentChunk()->setAt(offset, (jump >> 8) & 0xff);
  currentChunk()->setAt(offset + 1, jump & 0xff);
}

uint8_t Compiler::parseVariable(const char *errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);

  declareVariable();
  if (scopeDepth_) return 0;

  return identifierConstant(parser_.previous);
}

void Compiler::declareVariable() {
  if (!scopeDepth_) return;

  Token &name = parser_.previous;

  for (int i = locals_.size() - 1; i >= 0; i--) {
    LocalVariable &local = locals_[i];
    if (local.depth != -1 && local.depth < scopeDepth_) break;

    if (identifiersEqual(name, local.name))
      error("Already a variable with this name in this scope.");
  }

  addLocal(name);
}

void Compiler::addLocal(const Token &name) {
  locals_.push_back(LocalVariable{name, -1});
};

uint8_t Compiler::identifierConstant(const Token &name) {
  return makeConstant(new AsasString(std::string(name.start, name.length).c_str()));
}

void Compiler::markInitialized() {
  if (scopeDepth_ == 0) return;
  locals_.back().depth = scopeDepth_;
}

void Compiler::defineVariable(uint8_t global) {
  if (scopeDepth_) return void(markInitialized());
  emitBytes(OP_DEFINE_GLOBAL, global);
}

void Compiler::statement() {
  if (match(TOKEN_PRINT)) printStatement();
  else if (match(TOKEN_IF)) ifStatement();
  else if (match(TOKEN_WHILE)) whileStatement();
  else if (match(TOKEN_FOR)) forStatement();
  else if (match(TOKEN_RETURN)) returnStatement();
  else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  }
  else expressionStatement();
}

void Compiler::returnStatement() {
  if (currentFunctionType_ == FunctionType::SCRIPT)
    error("Can't return from top-level code.");

  if (match(TOKEN_SEMICOLON)) {
    emitByte(OP_NIL);
    emitByte(OP_RETURN);
  } else {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
    emitByte(OP_RETURN);
  }
}

void Compiler::forStatement() {
  beginScope();
  // Initializer clause.
  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
  if (match(TOKEN_VAR)) varDeclaration();
  else if (!match(TOKEN_SEMICOLON)) expressionStatement();

  // Condition clause.
  int loopStart = currentChunk()->getCode().size();
  int exitJump = -1;
  if (!match(TOKEN_SEMICOLON)) {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
  }

  // Increment clause.
  if (!match(TOKEN_RIGHT_PAREN)) {
    int bodyJump = emitJump(OP_JUMP);
    int incrementStart = currentChunk()->getCode().size();
    expression();
    emitByte(OP_POP);
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    emitLoop(loopStart);
    loopStart = incrementStart;
    patchJump(bodyJump);
  } 

  statement();
  emitLoop(loopStart);
  if (exitJump != -1) {
    patchJump(exitJump);
    emitByte(OP_POP);
  }
  endScope();
}

void Compiler::whileStatement() {
  int loopStart = currentChunk()->getCode().size();
  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after 'while'.");

  int exitJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();
  emitLoop(loopStart);

  patchJump(exitJump);
  emitByte(OP_POP);
}

void Compiler::emitLoop(int loopStart) {
  emitByte(OP_LOOP);

  int offset = currentChunk()->getCode().size() - loopStart + 2;
  if (offset > 0xffff) error("Loop body too large.");

  emitByte((offset >> 8) & 0xff);
  emitByte(offset & 0xff);
}

void Compiler::block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::printStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

void Compiler::expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

void Compiler::expression() {
  parsePrecedence(PREC_ASSIGNMENT);
}

void Compiler::parsePrecedence(Precedence precedence) {
  advance();
  ParseFn prefixRule = ParseRule::getRule(parser_.previous.type)->prefix;
  if (prefixRule == nullptr) {
    error("Expect expression.");
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  (this->*prefixRule)(canAssign);

  while (precedence <= ParseRule::getRule(parser_.current.type)->precedence) {
    advance();
    ParseFn infixRule = ParseRule::getRule(parser_.previous.type)->infix;
    (this->*infixRule)(canAssign);
  }
}

void Compiler::call(bool) {
  uint8_t argCount = argumentsList();
  emitBytes(OP_CALL, argCount);
}

uint8_t Compiler::argumentsList() {
  uint8_t argCount = 0;
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      expression();
      if (argCount == 255) error("Can't have more than 255 arguments.");
      argCount++;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  return argCount;
}

void Compiler::variable(bool canAssign) {
  namedVariable(parser_.previous, canAssign);
}

void Compiler::namedVariable(const Token &name, bool canAssign) {
  uint8_t getOp, setOp;
  int argumentIndex = resolveLocal(name);
  if (argumentIndex != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else if ((argumentIndex = resolveUpvalue(name)) != -1) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
  }
  else {
    argumentIndex = identifierConstant(name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, (uint8_t)argumentIndex);
  } else emitBytes(getOp, (uint8_t)argumentIndex);
}

int Compiler::resolveUpvalue(const Token &name) {
  if (enclosing_ == nullptr) return -1;

  int localIndex = enclosing_->resolveLocal(name);
  if (localIndex != -1) {
    enclosing_->locals_[localIndex].isCaptured = true;
    return addUpvalue((uint8_t)localIndex, true);
  }

  int upvalueIndex = enclosing_->resolveUpvalue(name);
  if (upvalueIndex != -1)
    return addUpvalue((uint8_t)upvalueIndex, false);

  return -1;
}

int Compiler::addUpvalue(uint8_t index, bool isLocal) {
  for (int i = 0; i < upvalues_.size(); i++)
    if (upvalues_[i].index == index && upvalues_[i].isLocal == isLocal)
      return i;

  if (upvalues_.size() == 256) {
    error("Too many closure variables in function.");
    return 0;
  }

  upvalues_.push_back(Upvalue(index, isLocal));
  currentFunction_->incrementUpvalueCount();

  return upvalues_.size() - 1;
}

int Compiler::resolveLocal(const Token &name) {
  for (int i = locals_.size() - 1; i >= 0; i--)
    if (locals_[i].depth != -1 && identifiersEqual(name, locals_[i].name)) {
      if (locals_[i].depth == -1)
        error("Can't read local variable in its own initializer.");

      return i;
    }

  return -1;
}

void Compiler::string(bool) {
  // Trim the surrounding quotes.
  std::string str(parser_.previous.start + 1, parser_.previous.length - 2);
  AsasString* stringObj = new AsasString(str.c_str());
  emitConstant(stringObj);
}

void Compiler::literal(bool) {
  switch (parser_.previous.type) {
  case TOKEN_FALSE: emitByte(OP_FALSE); break;
  case TOKEN_NIL: emitByte(OP_NIL); break;
  case TOKEN_TRUE: emitByte(OP_TRUE); break;
  default: return; // Unreachable.
  }
}

void Compiler::number(bool) {
  double value = strtod(parser_.previous.start, nullptr);
  emitBytes(OP_CONSTANT, currentChunk()->addConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
  int constant = currentChunk()->addConstant(value);
  if (constant > 255) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return (uint8_t)constant;
}

void Compiler::grouping(bool) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::andOperator(bool) {
  int endJump = emitJump(OP_JUMP_IF_FALSE);

  emitByte(OP_POP);
  parsePrecedence(PREC_AND);

  patchJump(endJump);
}

void Compiler::orOperator(bool) {
  int elseJump = emitJump(OP_JUMP_IF_FALSE);
  int endJump = emitJump(OP_JUMP);

  patchJump(elseJump);
  emitByte(OP_POP);

  parsePrecedence(PREC_OR);
  patchJump(endJump);
}

void Compiler::unary(bool) {
  TokenType operatorType = parser_.previous.type;

  // Compile the operand.
  parsePrecedence(PREC_UNARY);

  // Emit the operator instruction.
  switch (operatorType) {
  case TOKEN_MINUS: emitByte(OP_NEGATE); break;
  case TOKEN_BANG: emitByte(OP_NOT); break;
  default: return; // Unreachable.
  }
}

void Compiler::binary(bool) {
  TokenType operatorType = parser_.previous.type;
  ParseRule *rule = ParseRule::getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType) {
  case TOKEN_BANG_EQUAL: emitBytes(OP_EQUAL, OP_NOT); break;
  case TOKEN_EQUAL_EQUAL: emitByte(OP_EQUAL); break;
  case TOKEN_GREATER: emitByte(OP_GREATER); break;
  case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
  case TOKEN_LESS: emitByte(OP_LESS); break;
  case TOKEN_LESS_EQUAL: emitBytes(OP_GREATER, OP_NOT); break;
  case TOKEN_PLUS: emitByte(OP_ADD); break;
  case TOKEN_MINUS: emitByte(OP_SUBTRACT); break;
  case TOKEN_STAR: emitByte(OP_MULTIPLY); break;
  case TOKEN_SLASH: emitByte(OP_DIVIDE); break;
  default: return; // Unreachable.
  }
}

void Compiler::advance() {
  parser_.previous = parser_.current;

  for (;;) {
    parser_.current = scanner_.scanToken();
    if (parser_.current.type != TOKEN_ERROR) break;

    errorAtCurrent(parser_.current.start);
  }
}

void Compiler::errorAt(const Token &token, const char *message) {
  if (parser_.panicMode) return;
  parser_.panicMode = true;
  parser_.hadError = true;
  fprintf(stderr, "[line %d] Error", token.line);

  if (token.type == TOKEN_EOF) fprintf(stderr, " at end");
  else if (token.type == TOKEN_ERROR)
    ; // nothing
  else
    printf(" at '%.*s'", token.length, token.start);

  fprintf(stderr, ": %s\n", message);
}

void Compiler::beginScope() {
  scopeDepth_++;
}

void Compiler::endScope() {
  scopeDepth_--;

  while (!locals_.empty() && locals_.back().depth > scopeDepth_) {
    if (locals_.back().isCaptured) emitByte(OP_CLOSE_UPVALUE);
    else emitByte(OP_POP);

    locals_.pop_back();
  }
}

void Compiler::synchronize() {
  parser_.panicMode = false;

  while (parser_.current.type != TOKEN_EOF) {
    if (parser_.previous.type == TOKEN_SEMICOLON) return;

    switch (parser_.current.type) {
    case TOKEN_CLASS:
    case TOKEN_FUNC:
    case TOKEN_VAR:
    case TOKEN_FOR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_PRINT:
    case TOKEN_RETURN:
      return;
    default:
      ; // Do nothing.
    }

    advance();
  }
}
