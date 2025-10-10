#include "scanner.h"
#include <string.h>

Token Scanner::scanToken() {
  skipWhitespace();

  start_ = current_;
  if (isAtEnd())
    return makeToken(TOKEN_EOF);

  char c = nextChar();
  if (isDigit(c))
    return number();
  if (isAlpha(c))
    return identifier();
  switch (c) {
  // Single-character tokens.
  case '(':
    return makeToken(TOKEN_LEFT_PAREN);
  case ')':
    return makeToken(TOKEN_RIGHT_PAREN);
  case '{':
    return makeToken(TOKEN_LEFT_BRACE);
  case '}':
    return makeToken(TOKEN_RIGHT_BRACE);
  case ';':
    return makeToken(TOKEN_SEMICOLON);
  case ',':
    return makeToken(TOKEN_COMMA);
  case '.':
    return makeToken(TOKEN_DOT);
  case '-':
    return makeToken(TOKEN_MINUS);
  case '+':
    return makeToken(TOKEN_PLUS);
  case '/':
    return makeToken(TOKEN_SLASH);
  case '*':
    return makeToken(TOKEN_STAR);

  // One or two character tokens.
  case '!':
    return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
  case '=':
    return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
  case '<':
    return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
  case '>':
    return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

  // Literals.
  case '"':
    return string();
  }

  return errorToken("Unexpected character.");
}

Token Scanner::string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      line_++;
    nextChar();
  }

  if (isAtEnd())
    return errorToken("Unterminated string.");

  // The closing ".
  nextChar();
  return makeToken(TOKEN_STRING);
}

Token Scanner::number() {
  while (isDigit(peek()))
    nextChar();

  // Look for a fractional part.
  if (peek() == '.' && isDigit(peekNext())) {
    // Consume the "."
    nextChar();

    while (isDigit(peek()))
      nextChar();
  }

  return makeToken(TOKEN_NUMBER);
}

Token Scanner::identifier() {
  while (isAlpha(peek()) || isDigit(peek()))
    nextChar();
  return makeToken(identifierType());
}

TokenType Scanner::identifierType() {
  switch (*start_) {
  case 'a':
    return checkKeyword(1, 2, "nd", TOKEN_AND);
  case 'c':
    return checkKeyword(1, 4, "lass", TOKEN_CLASS);
  case 'e':
    return checkKeyword(1, 3, "lse", TOKEN_ELSE);
  case 'f':
    if (current_ - start_ > 1) {
      switch (*(start_ + 1)) {
      case 'a':
        return checkKeyword(2, 3, "lse", TOKEN_FALSE);
      case 'o':
        return checkKeyword(2, 1, "r", TOKEN_FOR);
      case 'u':
        return checkKeyword(2, 1, "n", TOKEN_FUN);
      }
    }
    break;
  case 'i':
    return checkKeyword(1, 1, "f", TOKEN_IF);
  case 'n':
    return checkKeyword(1, 2, "il", TOKEN_NIL);
  case 'o':
    return checkKeyword(1, 1, "r", TOKEN_OR);
  case 'p':
    return checkKeyword(1, 4, "rint", TOKEN_PRINT);
  case 'r':
    return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
  case 's':
    return checkKeyword(1, 4, "uper", TOKEN_SUPER);
  case 't':
    if (current_ - start_ > 1) {
      switch (*(start_ + 1)) {
      case 'h':
        return checkKeyword(2, 2, "is", TOKEN_THIS);
      case 'r':
        return checkKeyword(2, 2, "ue", TOKEN_TRUE);
      }
    }
    break;
  case 'v':
    return checkKeyword(1, 2, "ar", TOKEN_VAR);
  case 'w':
    return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }
  return TOKEN_IDENTIFIER;
}

TokenType Scanner::checkKeyword(int start, int length, const char *rest,
                                TokenType type) {
  if (current_ - this->start_ == start + length &&
      memcmp(this->start_ + start, rest, length) == 0) {
    return type;
  }
  return TOKEN_IDENTIFIER;
}

Token Scanner::makeToken(TokenType type) {
  return Token(type, start_, (int)(current_ - start_), line_);
}

Token Scanner::errorToken(const char *message) {
  return Token(TOKEN_ERROR, message, (int)strlen(message), line_);
}

void Scanner::skipWhitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      nextChar();
      break;
    case '\n':
      line_++;
      nextChar();
      break;
    case '/':
      if (peekNext() == '/') {
        // A comment goes until the end of the line.
        while (peek() != '\n' && !isAtEnd())
          nextChar();
      } else {
        return;
      }
      break;
    default:
      return;
    }
  }
}
