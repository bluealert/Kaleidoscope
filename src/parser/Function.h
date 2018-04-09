#pragma once

#include <llvm/IR/Function.h>
#include <string>
#include <vector>
#include "parser/Expr.h"

namespace kaso {
namespace parser {

class Prototype {
 public:
  Prototype(std::string name, std::vector<std::string> args,
            bool isOperator = false, lexer::Token op = lexer::Token::Error,
            uint32_t prec = 0)
      : name_(std::move(name)),
        args_(std::move(args)),
        isOperator_(isOperator),
        op_(op),
        precedence_(prec) {}

  llvm::Function* codeGen();

  const std::string getName() const { return name_; }

  bool isUnaryOp() const { return isOperator_ && args_.size() == 1; }
  bool isBinaryOp() const { return isOperator_ && args_.size() == 2; }

  lexer::Token getOperator() const { return op_; }
  uint32_t getBinOpPrecedence() const { return precedence_; }

 private:
  std::string name_;
  std::vector<std::string> args_;
  bool isOperator_;
  lexer::Token op_;
  uint32_t precedence_;
};

class Function {
 public:
  Function(std::unique_ptr<Prototype> proto, std::unique_ptr<Expr> body)
      : proto_(std::move(proto)), body_(std::move(body)) {}

  llvm::Function* codeGen();

 private:
  std::unique_ptr<Prototype> proto_;
  std::unique_ptr<Expr> body_;
};

}  // namespace parser
}  // namespace kaso
