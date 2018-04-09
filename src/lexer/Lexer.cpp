#include "lexer/Lexer.h"
#include <map>

namespace kaso {
namespace lexer {

Lexer::Lexer(std::istream& is) : lastChar_(' '), numVal_(0.0), is_(is) {}

Token Lexer::getTok() {
  while (std::isspace(lastChar_)) {
    lastChar_ = is_.get();
  }

  auto tok = Token::Error;
  switch (lastChar_) {
    case ',':
      tok = Token::Comma;
      break;
    case ';':
      tok = Token::Semicolon;
      break;
    case '(':
      tok = Token::LeftParen;
      break;
    case ')':
      tok = Token::RightParen;
      break;
    case '=':
      lastChar_ = is_.get();
      if (lastChar_ == '=') {
        tok = Token::OpEQ;
        strVal_ = "==";
      } else {
        tok = Token::OpAssign;
        strVal_ = "=";
        is_.unget();
      }
      break;
    case '>':
      lastChar_ = is_.get();
      if (lastChar_ == '=') {
        tok = Token::OpGE;
        strVal_ = ">=";
      } else {
        tok = Token::OpGreat;
        strVal_ = ">";
        is_.unget();
      }
      break;
    case '<':
      lastChar_ = is_.get();
      if (lastChar_ == '=') {
        tok = Token::OpLE;
        strVal_ = "<=";
      } else {
        tok = Token::OpLess;
        strVal_ = "<";
        is_.unget();
      }
      break;
    case ':':
      tok = Token::OpColon;
      strVal_ = ":";
      break;
    case '&':
      lastChar_ = is_.get();
      if (lastChar_ == '&') {
        tok = Token::OpLogicAnd;
        strVal_ = "&&";
        break;
      } else {
        return Token::Error;
      }
    case '|':
      lastChar_ = is_.get();
      if (lastChar_ == '|') {
        tok = Token::OpLogicOr;
        strVal_ = "||";
        break;
      } else {
        return Token::Error;
      }
    case '+':
      tok = Token::OpAdd;
      strVal_ = "+";
      break;
    case '-':
      tok = Token::OpSub;
      strVal_ = "-";
      break;
    case '*':
      tok = Token::OpMul;
      strVal_ = "*";
      break;
    case '/':
      tok = Token::OpDiv;
      strVal_ = "/";
      break;
    case '!':
      tok = Token::OpNegate;
      strVal_ = "!";
      break;
    default:
      break;
  }
  if (tok != Token::Error) {
    lastChar_ = is_.get();
    return tok;
  }

  if (std::isalpha(lastChar_)) {
    strVal_ = static_cast<char>(lastChar_);
    while (std::isalnum((lastChar_ = is_.get()))) {
      strVal_ += static_cast<char>(lastChar_);
    }

    if (strVal_ == "def") {
      return Token::Def;
    }
    if (strVal_ == "extern") {
      return Token::Extern;
    }
    if (strVal_ == "if") {
      return Token::If;
    }
    if (strVal_ == "then") {
      return Token::Then;
    }
    if (strVal_ == "else") {
      return Token::Else;
    }
    if (strVal_ == "for") {
      return Token::For;
    }
    if (strVal_ == "in") {
      return Token::In;
    }
    if (strVal_ == "binary") {
      return Token::Binary;
    }
    if (strVal_ == "unary") {
      return Token::Unary;
    }
    return Token::Identifier;
  }

  if (std::isdigit(lastChar_) || lastChar_ == '.') {
    std::string numStr;
    do {
      numStr += static_cast<char>(lastChar_);
      lastChar_ = is_.get();
    } while (std::isdigit(lastChar_) || lastChar_ == '.');

    numVal_ = std::stod(numStr);
    return Token::Number;
  }

  if (lastChar_ == '#') {
    do {
      lastChar_ = is_.get();
    } while (lastChar_ != EOF && lastChar_ != '\n' && lastChar_ != '\r');

    if (lastChar_ != EOF) {
      return getTok();
    }
  }

  if (lastChar_ == EOF) {
    return Token::Eof;
  }

  return Token::Error;
}

double Lexer::numVal() { return numVal_; }

std::string Lexer::strVal() { return strVal_; }

bool isValidUnaryOperator(Token tok) {
  static const std::map<Token, bool> validUnary = {{Token::OpNegate, true},
                                                   {Token::OpSub, true}};
  return validUnary.find(tok) != validUnary.end();
}

bool isValidBinaryOperator(Token tok) {
  static const std::map<Token, bool> validBinary = {
      {Token::OpAssign, true},   {Token::OpEQ, true},
      {Token::OpNE, true},       {Token::OpLess, true},
      {Token::OpGreat, true},    {Token::OpLE, true},
      {Token::OpGE, true},       {Token::OpColon, true},
      {Token::OpLogicAnd, true}, {Token::OpLogicOr, true},
      {Token::OpAdd, true},      {Token::OpSub, true},
      {Token::OpMul, true},      {Token::OpDiv, true}};
  return validBinary.find(tok) != validBinary.end();
}

}  // namespace lexer
}  // namespace kaso
