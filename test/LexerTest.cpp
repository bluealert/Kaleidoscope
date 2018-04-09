#include <gtest/gtest.h>
#include "lexer/Lexer.h"

namespace kaso {
namespace lexer {

namespace {
const std::map<std::string, std::vector<lexer::Token>> data = {
    {"3 > 4;",
     {Token::Number, Token::OpGreat, Token::Number, Token::Semicolon}},

    {"def foo(x y) x+foo(y, 4.0);",
     {Token::Def, Token::Identifier, Token::LeftParen, Token::Identifier,
      Token::Identifier, Token::RightParen, Token::Identifier, Token::OpAdd,
      Token::Identifier, Token::LeftParen, Token::Identifier, Token::Comma,
      Token::Number, Token::RightParen, Token::Semicolon}},

    {"def foo(x y) x+y y;",
     {Token::Def, Token::Identifier, Token::LeftParen, Token::Identifier,
      Token::Identifier, Token::RightParen, Token::Identifier, Token::OpAdd,
      Token::Identifier, Token::Identifier, Token::Semicolon}},

    {"extern sin(a);",
     {Token::Extern, Token::Identifier, Token::LeftParen, Token::Identifier,
      Token::RightParen, Token::Semicolon}},

    {"def foo(x y) x+y );",
     {Token::Def, Token::Identifier, Token::LeftParen, Token::Identifier,
      Token::Identifier, Token::RightParen, Token::Identifier, Token::OpAdd,
      Token::Identifier, Token::RightParen, Token::Semicolon}},

    {"def binary || 5 (lhs rhs) if lhs then 1 else if rhs then 1 else 0;",
     {Token::Def,        Token::Binary,     Token::OpLogicOr,
      Token::Number,     Token::LeftParen,  Token::Identifier,
      Token::Identifier, Token::RightParen, Token::If,
      Token::Identifier, Token::Then,       Token::Number,
      Token::Else,       Token::If,         Token::Identifier,
      Token::Then,       Token::Number,     Token::Else,
      Token::Number,     Token::Semicolon}},

    {"def binary||5(lhs rhs)if lhs then 1 else if rhs then 1 else 0;",
     {Token::Def,        Token::Binary,     Token::OpLogicOr,
      Token::Number,     Token::LeftParen,  Token::Identifier,
      Token::Identifier, Token::RightParen, Token::If,
      Token::Identifier, Token::Then,       Token::Number,
      Token::Else,       Token::If,         Token::Identifier,
      Token::Then,       Token::Number,     Token::Else,
      Token::Number,     Token::Semicolon}},

    {"def binary : 1 (x y) 0;",
     {Token::Def, Token::Binary, Token::OpColon, Token::Number,
      Token::LeftParen, Token::Identifier, Token::Identifier, Token::RightParen,
      Token::Number, Token::Semicolon}},

    {"def unary!(v) if v then 0 else 1;",
     {Token::Def, Token::Unary, Token::OpNegate, Token::LeftParen,
      Token::Identifier, Token::RightParen, Token::If, Token::Identifier,
      Token::Then, Token::Number, Token::Else, Token::Number,
      Token::Semicolon}}};
}

void assertTokens(lexer::Lexer& lex, const std::vector<lexer::Token>& tokens1) {
  std::vector<lexer::Token> tokens2;
  lexer::Token tok;
  while ((tok = lex.getTok()) != Token::Eof) {
    tokens2.push_back(tok);
  }

  ASSERT_EQ(tokens1.size(), tokens2.size());
  for (auto i = 0; i < tokens1.size(); i++) {
    ASSERT_EQ(tokens1[i], tokens2[i]);
  }
}

TEST(GetTokenTest, GetToken) {
  size_t n = 1;
  ASSERT_EQ(1, n);
  for (const auto& kv : data) {
    std::stringstream ss(kv.first);
    Lexer lex(ss);
    assertTokens(lex, kv.second);
  }
}

}  // namespace lexer
}  // namespace kaso
