#include <cstdlib>
#include "chunk.h"
#include "stdio.h"

#include "parse_rule.h"
#include "compiler.h"
#include "scanner.h"


bool Compiler::compile() {
  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");
  emitByte(OP_RETURN);
  return !parser_.hadError;


  // [TODO] - Remember
  // int line = 1;
  // for (;;) {
  //   Token token = scanner.scanToken();
  //   if (token.line != line) {
  //     printf("%4d ", token.line);
  //     line = token.line;
  //   } else printf("   | ");
  //
  //   printf("%2d '%.*s'\n", token.type, token.length, token.start);
  //
  //   if (token.type == TOKEN_EOF) return true;
  // }
  // return false;
}

void Compiler::advance() {
  parser_.previous = parser_.current;

  for (;;) {
    parser_.current = scanner_.scanToken();
    if (parser_.current.type != TOKEN_ERROR) break;

    errorAtCurrent(parser_.current.start);
  }
}

void Compiler::consume(TokenType type, const char *message) {
  if (parser_.current.type == type) return advance();
  errorAtCurrent(message);
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

  (this->*prefixRule)();

  while (precedence <= ParseRule::getRule(parser_.current.type)->precedence) {
    advance();
    ParseFn infixRule = ParseRule::getRule(parser_.previous.type)->infix;
    (this->*infixRule)();
  }
}

void Compiler::literal() {
  switch (parser_.previous.type) {
  case TOKEN_FALSE: emitByte(OP_FALSE); break;
  case TOKEN_NIL: emitByte(OP_NIL); break;
  case TOKEN_TRUE: emitByte(OP_TRUE); break;
  default: return; // Unreachable.
  }
}

void Compiler::number() {
  double value = strtod(parser_.previous.start, nullptr);
  emitBytes(OP_CONSTANT, chunk_.addConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
  int constant = currentChunk().addConstant(value);
  if (constant > 255) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return (uint8_t)constant;
}

void Compiler::grouping() {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary() {
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

void Compiler::binary() {
  TokenType operatorType = parser_.previous.type;
  ParseRule *rule = ParseRule::getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType) {
  case TOKEN_PLUS: emitByte(OP_ADD); break;
  case TOKEN_MINUS: emitByte(OP_SUBTRACT); break;
  case TOKEN_STAR: emitByte(OP_MULTIPLY); break;
  case TOKEN_SLASH: emitByte(OP_DIVIDE); break;
  default: return; // Unreachable.
  }
}
