#pragma once

#include <llvm/IR/Value.h>
#include <utility>
#include <vector>
#include "lexer/Lexer.h"

namespace kaso {
namespace parser {

class Expr {
 public:
  virtual ~Expr() = default;
  virtual llvm::Value* codeGen() = 0;
};

class NumberExpr : public Expr {
 public:
  explicit NumberExpr(double val) : val_(val) {}

  llvm::Value* codeGen() override;

 private:
  double val_;
};

class VariableExpr : public Expr {
 public:
  explicit VariableExpr(std::string name) : name_(std::move(name)) {}

  llvm::Value* codeGen() override;

 private:
  std::string name_;
};

class BinaryExpr : public Expr {
 public:
  BinaryExpr(lexer::Token op, std::string opStr, std::unique_ptr<Expr> lhs,
             std::unique_ptr<Expr> rhs)
      : op_(op),
        opStr_(std::move(opStr)),
        lhs_(std::move(lhs)),
        rhs_(std::move(rhs)) {}

  llvm::Value* codeGen() override;

 private:
  lexer::Token op_;
  std::string opStr_;
  std::unique_ptr<Expr> lhs_, rhs_;
};

class CallExpr : public Expr {
 public:
  CallExpr(std::string callee, std::vector<std::unique_ptr<Expr>> args)
      : callee_(std::move(callee)), args_(std::move(args)) {}

  llvm::Value* codeGen() override;

 private:
  std::string callee_;
  std::vector<std::unique_ptr<Expr>> args_;
};

class IfExpr : public Expr {
 public:
  IfExpr(std::unique_ptr<Expr> cond, std::unique_ptr<Expr> then,
         std::unique_ptr<Expr> els)
      : cond_(std::move(cond)), then_(std::move(then)), else_(std::move(els)) {}

  llvm::Value* codeGen() override;

 private:
  std::unique_ptr<Expr> cond_, then_, else_;
};

class ForExpr : public Expr {
 public:
  ForExpr(std::string varName, std::unique_ptr<Expr> start,
          std::unique_ptr<Expr> end, std::unique_ptr<Expr> step,
          std::unique_ptr<Expr> body)
      : varName_(std::move(varName)),
        start_(std::move(start)),
        end_(std::move(end)),
        step_(std::move(step)),
        body_(std::move(body)){};

  // Output for-loop as:
  //   ...
  //   start = startexpr
  //   goto loop
  // loop:
  //   variable = phi [start, loopheader], [nextvariable, loopend]
  //   ...
  //   bodyexpr
  //   ...
  // loopend:
  //   step = stepexpr
  //   nextvariable = variable + step
  //   endcond = endexpr
  //   br endcond, loop, endloop
  // outloop:
  llvm::Value* codeGen() override;

 private:
  std::string varName_;
  std::unique_ptr<Expr> start_, end_, step_, body_;
};

class UnaryExpr : public Expr {
 public:
  UnaryExpr(lexer::Token op, std::string opStr, std::unique_ptr<Expr> operand)
      : op_(op), opStr_(std::move(opStr)), operand_(std::move(operand)) {}

  llvm::Value* codeGen() override;

 private:
  lexer::Token op_;
  std::string opStr_;
  std::unique_ptr<Expr> operand_;
};

}  // namespace parser
}  // namespace kaso
