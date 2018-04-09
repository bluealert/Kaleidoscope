#pragma once

#include <istream>
#include <string>

namespace kaso {
namespace lexer {

enum class Token {
  Error,
  Eof,

  Def,         // def
  Extern,      // extern
  Identifier,  //
  Number,      //

  If,      // if
  Then,    // then
  Else,    // else
  For,     // for
  In,      // in
  Binary,  // binary
  Unary,   // unary

  Comma,       // ,
  Semicolon,   // ;
  LeftParen,   // (
  RightParen,  // )

  OpAssign,  // =
  OpEQ,      // ==
  OpNE,      // !=
  OpLess,    // <
  OpGreat,   // >
  OpLE,      // <=
  OpGE,      // >=

  OpColon,  // :

  OpLogicAnd,  // &&
  OpLogicOr,   // ||

  OpAdd,  // +
  OpSub,  // -, also can be a unary negation operator
  OpMul,  // *
  OpDiv,  // /

  OpNegate,  // !
};

class Lexer {
 public:
  explicit Lexer(std::istream& is);

  Token getTok();

  double numVal();

  std::string strVal();

 private:
  std::istream& is_;
  int lastChar_;
  double numVal_;
  std::string strVal_;
};

bool isValidUnaryOperator(Token tok);
bool isValidBinaryOperator(Token tok);

}  // namespace lexer
}  // namespace kaso
