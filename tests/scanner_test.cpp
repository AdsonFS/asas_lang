#include "scanner.h"
#include <gtest/gtest.h>

TEST(ScannerTest, LiteralsTokens) {
  const char *source =
      "\"hello\" 12.42 count \"world\"3 name \"test string\"  ";
  Scanner scanner(source);
  std::vector<std::pair<TokenType, const char*>> expectedTokens = {
      {TOKEN_STRING, "\"hello\""},      {TOKEN_NUMBER, "12.42"},
      {TOKEN_IDENTIFIER, "count"},  {TOKEN_STRING, "\"world\""},
      {TOKEN_NUMBER, "3"},          {TOKEN_IDENTIFIER, "name"},
      {TOKEN_STRING, "\"test string\""}};
  for (const auto &[expType, expStr] : expectedTokens) {
    Token token = scanner.scanToken();
    EXPECT_EQ(token.type, expType);
    EXPECT_STREQ(std::string(token.start, token.length).c_str(), expStr);
  }

}

TEST(ScannerTest, OperatorsTokens) {
  const char *source = "= == > >= < <= ! != + - * / ( ) { } , . ;";
  Scanner scanner(source);
  std::vector<std::pair<TokenType, const char*>> expectedTokens = {
      {TOKEN_EQUAL, "="},         {TOKEN_EQUAL_EQUAL, "=="},
      {TOKEN_GREATER, ">"},       {TOKEN_GREATER_EQUAL, ">="},
      {TOKEN_LESS, "<"},          {TOKEN_LESS_EQUAL, "<="},
      {TOKEN_BANG, "!"},          {TOKEN_BANG_EQUAL, "!="},
      {TOKEN_PLUS, "+"},          {TOKEN_MINUS, "-"},
      {TOKEN_STAR, "*"},          {TOKEN_SLASH, "/"},
      {TOKEN_LEFT_PAREN, "("},    {TOKEN_RIGHT_PAREN, ")"},
      {TOKEN_LEFT_BRACE, "{"},    {TOKEN_RIGHT_BRACE, "}"},
      {TOKEN_COMMA, ","},         {TOKEN_DOT, "."},
      {TOKEN_SEMICOLON, ";"}};
  for (const auto &[expType, expStr] : expectedTokens) {
    Token token = scanner.scanToken();
    EXPECT_EQ(token.type, expType);
    EXPECT_STREQ(std::string(token.start, token.length).c_str(), expStr);
  }
}

TEST(ScannerTest, OperatorsTokensWithoutSpaces) {
  const char *source = "=+*/( ){},.;>!<=<";
  Scanner scanner(source);
  std::vector<std::pair<TokenType, const char*>> expectedTokens = {
      {TOKEN_EQUAL, "="},         {TOKEN_PLUS, "+"},
      {TOKEN_STAR, "*"},          {TOKEN_SLASH, "/"},
      {TOKEN_LEFT_PAREN, "("},    {TOKEN_RIGHT_PAREN, ")"},
      {TOKEN_LEFT_BRACE, "{"},    {TOKEN_RIGHT_BRACE, "}"},
      {TOKEN_COMMA, ","},         {TOKEN_DOT, "."},
      {TOKEN_SEMICOLON, ";"},     {TOKEN_GREATER, ">"},
      {TOKEN_BANG, "!"},          {TOKEN_LESS_EQUAL, "<="},
      {TOKEN_LESS, "<"}};
  for (const auto &[expType, expStr] : expectedTokens) {
    Token token = scanner.scanToken();
    EXPECT_EQ(token.type, expType);
    EXPECT_STREQ(std::string(token.start, token.length).c_str(), expStr);
  }
}


