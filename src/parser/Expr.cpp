#include "parser/Expr.h"
#include "global/Global.h"

namespace kaso {
namespace parser {

llvm::Value* NumberExpr::codeGen() {
  return llvm::ConstantFP::get(global::gContext(), llvm::APFloat(val_));
}

llvm::Value* VariableExpr::codeGen() {
  auto v = global::gNamedValues()[name_];
  if (!v) {
    return logErrorV("Unknown variable name");
  }
  return v;
}

llvm::Value* BinaryExpr::codeGen() {
  auto l = lhs_->codeGen();
  auto r = rhs_->codeGen();
  if (!l || !r) {
    return nullptr;
  }

  switch (op_) {
    case lexer::Token::OpAdd:
      return global::gBuilder().CreateFAdd(l, r, "addtmp");
    case lexer::Token::OpSub:
      return global::gBuilder().CreateFSub(l, r, "subtmp");
    case lexer::Token::OpMul:
      return global::gBuilder().CreateFMul(l, r, "multmp");
    case lexer::Token::OpLess: {
      l = global::gBuilder().CreateFCmpULT(l, r, "cmptmp");
      auto destTy = llvm::Type::getDoubleTy(global::gContext());
      return global::gBuilder().CreateUIToFP(l, destTy, "bootmp");
    }
    default:
      break;
  }

  auto f = global::getFunction(std::string("binary") + opStr_);
  assert(f && "binary operator not found!");

  // If it wasn't a builtin binary operator, it must be a user defined one. Emit
  // a call to it.
  llvm::ArrayRef<llvm::Value*> args = {l, r};
  return global::gBuilder().CreateCall(f, args, "binop");
}

llvm::Value* CallExpr::codeGen() {
  auto calleeF = global::getFunction(callee_);
  if (!calleeF) {
    return logErrorV("Unknown function referenced");
  }
  if (calleeF->arg_size() != args_.size()) {
    return logErrorV("Incorrect # arguments passed");
  }

  std::vector<llvm::Value*> argsV;
  for (size_t i = 0, e = args_.size(); i != e; ++i) {
    auto c = args_[i]->codeGen();
    if (c == nullptr) {
      return nullptr;
    }
    argsV.push_back(c);
  }

  return global::gBuilder().CreateCall(calleeF, argsV, "calltmp");
}

llvm::Value* IfExpr::codeGen() {
  auto condV = cond_->codeGen();
  if (condV == nullptr) {
    return nullptr;
  }

  auto c = llvm::ConstantFP::get(global::gContext(), llvm::APFloat(0.0));
  condV = global::gBuilder().CreateFCmpONE(condV, c, "ifcond");

  auto func = global::gBuilder().GetInsertBlock()->getParent();
  auto thenBb = llvm::BasicBlock::Create(global::gContext(), "then", func);
  auto elseBb = llvm::BasicBlock::Create(global::gContext(), "else");
  auto mergeBb = llvm::BasicBlock::Create(global::gContext(), "ifcont");
  global::gBuilder().CreateCondBr(condV, thenBb, elseBb);

  // emit then value
  global::gBuilder().SetInsertPoint(thenBb);
  auto thenV = then_->codeGen();
  if (thenV == nullptr) {
    return nullptr;
  }

  global::gBuilder().CreateBr(mergeBb);
  thenBb = global::gBuilder().GetInsertBlock();

  // emit the else block.
  func->getBasicBlockList().push_back(elseBb);
  global::gBuilder().SetInsertPoint(elseBb);
  auto elseV = else_->codeGen();
  if (elseV == nullptr) {
    return nullptr;
  }

  global::gBuilder().CreateBr(mergeBb);
  elseBb = global::gBuilder().GetInsertBlock();

  // emit the merge block.
  func->getBasicBlockList().push_back(mergeBb);
  global::gBuilder().SetInsertPoint(mergeBb);
  auto type = llvm::Type::getDoubleTy(global::gContext());
  auto phiNode = global::gBuilder().CreatePHI(type, 2, "iftmp");
  phiNode->addIncoming(thenV, thenBb);
  phiNode->addIncoming(elseV, elseBb);

  return phiNode;
}

llvm::Value* ForExpr::codeGen() {
  auto startVal = start_->codeGen();
  if (start_ == nullptr) {
    return nullptr;
  }

  auto func = global::gBuilder().GetInsertBlock()->getParent();
  auto preHeaderBb = global::gBuilder().GetInsertBlock();
  auto loopBb = llvm::BasicBlock::Create(global::gContext(), "loop", func);
  global::gBuilder().CreateBr(loopBb);

  // start insertion in loopBb.
  global::gBuilder().SetInsertPoint(loopBb);

  // start the phi node with an entry for start.
  auto type = llvm::Type::getDoubleTy(global::gContext());
  auto var = global::gBuilder().CreatePHI(type, 2, varName_);
  var->addIncoming(startVal, preHeaderBb);

  auto oldVal = global::gNamedValues()[varName_];
  global::gNamedValues()[varName_] = var;

  // emit the body of the loop.
  if (body_->codeGen() == nullptr) {
    return nullptr;
  }

  // emit the step value.
  llvm::Value* stepVal = nullptr;
  if (step_ != nullptr) {
    stepVal = step_->codeGen();
    if (stepVal == nullptr) {
      return nullptr;
    }
  } else {
    stepVal = llvm::ConstantFP::get(global::gContext(), llvm::APFloat(1.0));
  }

  auto nextVar = global::gBuilder().CreateFAdd(var, stepVal, "nextvar");

  auto endCond = end_->codeGen();
  if (endCond == nullptr) {
    return nullptr;
  }

  auto cons = llvm::ConstantFP::get(global::gContext(), llvm::APFloat(0.0));
  endCond = global::gBuilder().CreateFCmpONE(endCond, cons, "loopcond");

  auto loopEndBb = global::gBuilder().GetInsertBlock();
  auto afterBb =
      llvm::BasicBlock::Create(global::gContext(), "afterloop", func);

  global::gBuilder().CreateCondBr(endCond, loopBb, afterBb);

  global::gBuilder().SetInsertPoint(afterBb);

  var->addIncoming(nextVar, loopEndBb);

  // restore the unshadowed variable.
  if (oldVal) {
    global::gNamedValues()[varName_] = oldVal;
  } else {
    global::gNamedValues().erase(varName_);
  }

  // for expr always returns 0.0.
  return llvm::Constant::getNullValue(type);
}

llvm::Value* UnaryExpr::codeGen() {
  auto v = operand_->codeGen();
  if (v == nullptr) {
    return nullptr;
  }

  auto f = global::getFunction(std::string("unary") + opStr_);
  if (f == nullptr) {
    return logErrorV("Unknown unary operator");
  }

  return global::gBuilder().CreateCall(f, v, "unop");
}

}  // namespace parser
}  // namespace kaso