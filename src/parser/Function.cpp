#include "parser/Function.h"
#include <llvm/IR/Verifier.h>
#include "global/Global.h"

namespace kaso {
namespace parser {

llvm::Function* Prototype::codeGen() {
  auto type = llvm::Type::getDoubleTy(global::gContext());
  std::vector<llvm::Type*> doubles(args_.size(), type);
  auto ft = llvm::FunctionType::get(type, doubles, false);
  auto link = llvm::Function::ExternalLinkage;
  auto f = llvm::Function::Create(ft, link, name_, global::gModule().get());

  auto idx = 0;
  for (auto& arg : f->args()) {
    arg.setName(args_[idx++]);
  }

  return f;
}

llvm::Function* Function::codeGen() {
  auto& p = *proto_;
  global::storeProto(p.getName(), std::move(proto_));
  auto func = global::getFunction(p.getName());
  if (!func) {
    return nullptr;
  }

  if (p.isBinaryOp()) {
    global::setBinOpTokPrecedence(p.getOperator(), p.getBinOpPrecedence());
  }

  auto bb = llvm::BasicBlock::Create(global::gContext(), "entry", func);
  global::gBuilder().SetInsertPoint(bb);

  global::gNamedValues().clear();
  for (auto& arg : func->args()) {
    global::gNamedValues()[arg.getName()] = &arg;
  }

  auto retVal = body_->codeGen();
  if (retVal != nullptr) {
    global::gBuilder().CreateRet(retVal);
    llvm::verifyFunction(*func);
    global::gFPM()->run(*func);
    return func;
  }

  func->eraseFromParent();
  if (p.isBinaryOp()) {
    global::eraseBinOpTok(p.getOperator());
  }
  return nullptr;
}

}  // namespace parser
}  // namespace kaso