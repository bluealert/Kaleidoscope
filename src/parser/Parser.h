#pragma once

#include <llvm/Support/raw_ostream.h>
#include <map>
#include "global/Global.h"
#include "lexer/Lexer.h"
#include "parser/Expr.h"
#include "parser/Function.h"

namespace kaso {
namespace parser {

class Parser {
 public:
  explicit Parser(lexer::Lexer lex);

  /// numberexpr ::= number
  std::unique_ptr<Expr> numberExpr();

  /// parenexpr ::= '(' expression ')'
  std::unique_ptr<Expr> parenExpr();

  /// identifierexpr
  ///   ::= identifier
  ///   ::= identifier '(' expression* ')'
  std::unique_ptr<Expr> identifierExpr();

  /// primary
  ///   ::= identifierexpr
  ///   ::= numberexpr
  ///   ::= parenexpr
  std::unique_ptr<Expr> primary();

  /// expression
  ///   ::= primary binoprhs
  ///
  std::unique_ptr<Expr> expression();

  /// binoprhs
  ///   ::= ('+' primary)*
  std::unique_ptr<Expr> binOpRHS(int prec, std::unique_ptr<Expr> lhs);

  /// prototype
  ///   ::= id '(' id* ')'
  ///   ::= binary LETTER number? (id, id)
  std::unique_ptr<Prototype> prototype();

  /// definition ::= 'def' prototype expression
  std::unique_ptr<Function> definition();

  /// toplevelexpr ::= expression
  std::unique_ptr<Function> topLevelExpr();

  /// external ::= 'extern' prototype
  std::unique_ptr<Prototype> externDef();

  /// ifexpr ::= 'if' expression 'then' expression 'else' expression
  std::unique_ptr<Expr> ifExpr();

  /// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
  std::unique_ptr<Expr> forExpr();

  /// unary
  ///   ::= primary
  ///   ::= '!' unary
  std::unique_ptr<Expr> unary();

  lexer::Token getNextToken();

  lexer::Token curToken();

 private:
  lexer::Lexer lexer_;
  lexer::Token curTok_;
};

}  // namespace parser
}  // namespace kaso
