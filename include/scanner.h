#ifndef asas_scanner_h
#define asas_scanner_h

#define TOKEN_COUNT 40
enum TokenType {
  // Single-character tokens.
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SEMICOLON,
  TOKEN_SLASH,
  TOKEN_STAR,

  // One or two character tokens.
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,

  // Literals.
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_NUMBER,

  // Keywords.
  TOKEN_AND,
  TOKEN_CLASS,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_FUN,
  TOKEN_IF,
  TOKEN_NIL,
  TOKEN_OR,
  TOKEN_PRINT,
  TOKEN_RETURN,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_TRUE,
  TOKEN_VAR,
  TOKEN_WHILE,

  // End of file.
  TOKEN_ERROR,
  TOKEN_EOF
};

class Token {
public:
  Token() : type(TOKEN_ERROR), start(nullptr), length(0), line(0) {}
  Token(TokenType type, const char *start, int length, int line)
      : type(type), start(start), length(length), line(line) {}

  TokenType type;
  const char *start;
  int length;
  int line;
};

class Scanner {
public:
  Scanner(const char *source) : start_(source), current_(source), line_(1) {}
  Token scanToken();

  const char* getRemainingSource() const { return current_; }

private:
  const char *start_;
  const char *current_;
  int line_;

  Token makeToken(TokenType type);
  Token errorToken(const char *message);
  Token string();
  Token number();
  Token identifier();

  TokenType identifierType();
  TokenType checkKeyword(int start, int length, const char *rest, TokenType type);

  void skipWhitespace();

  bool isAtEnd() const { return *current_ == '\0'; }

  char peek() const { return *current_; }
  char peekNext() const { return isAtEnd() ? '\0' : *(current_ + 1); }
  char nextChar() { return *current_++; }
  bool match(char expected) {
    if (isAtEnd() || *current_ != expected)
      return false;
    return current_++;
  }
  bool isDigit(char c) const { return c >= '0' && c <= '9'; }
  bool isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }
};

#endif // asas_scanner_h
