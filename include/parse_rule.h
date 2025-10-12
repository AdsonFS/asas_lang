#ifndef asas_parse_rule_h
#define asas_parse_rule_h

#include "compiler.h"

typedef void (Compiler::*ParseFn)();
class ParseRule {
public:
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;

  static ParseRule *getRule(TokenType type);
  static ParseRule rules[TOKEN_COUNT];

private:
};

// ParseRule *getRule(TokenType type);

#endif // asas_parse_rule_h
