#include "Global.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

namespace kaso {

namespace {
std::unique_ptr<llvm::LLVMContext> gContext_;
std::unique_ptr<llvm::IRBuilder<>> gBuilder_;
std::unique_ptr<llvm::Module> gModule_;
std::map<std::string, llvm::Value*> gNamedValues_;
std::unique_ptr<llvm::legacy::FunctionPassManager> gFPM_;
std::unique_ptr<llvm::orc::KaleidoscopeJIT> gJIT_;
std::map<std::string, std::unique_ptr<parser::Prototype>> gFuncProtos_;

std::map<lexer::Token, int> binOpPrec = {{lexer::Token::OpLess, 10},
                                         {lexer::Token::OpAdd, 20},
                                         {lexer::Token::OpSub, 20},
                                         {lexer::Token::OpMul, 40}};
}  // namespace

std::unique_ptr<parser::Expr> logError(const char* s) {
  fprintf(stderr, "LogError: %s\n", s);
  return nullptr;
}

std::unique_ptr<parser::Prototype> logErrorP(const char* s) {
  logError(s);
  return nullptr;
}

llvm::Value* logErrorV(const char* s) {
  logError(s);
  return nullptr;
}

namespace global {

void init() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  gContext_ = std::make_unique<llvm::LLVMContext>();
  gBuilder_ = std::make_unique<llvm::IRBuilder<>>(*gContext_);
  gJIT_ = std::make_unique<llvm::orc::KaleidoscopeJIT>();
}

void initModuleAndPassManager() {
  gModule_ = std::make_unique<llvm::Module>("gModule", *gContext_);
  gModule_->setDataLayout(gJIT_->getTargetMachine().createDataLayout());

  gFPM_ = std::make_unique<llvm::legacy::FunctionPassManager>(gModule_.get());
  gFPM_->add(llvm::createInstructionCombiningPass());
  gFPM_->add(llvm::createReassociatePass());
  gFPM_->add(llvm::createGVNPass());
  gFPM_->add(llvm::createCFGSimplificationPass());
  gFPM_->doInitialization();
}

llvm::LLVMContext& gContext() { return *gContext_; }

llvm::IRBuilder<>& gBuilder() { return *gBuilder_; }

std::unique_ptr<llvm::Module>& gModule() { return gModule_; }

std::map<std::string, llvm::Value*>& gNamedValues() { return gNamedValues_; }

std::unique_ptr<llvm::legacy::FunctionPassManager>& gFPM() { return gFPM_; }

std::unique_ptr<llvm::orc::KaleidoscopeJIT>& gJIT() { return gJIT_; };

void storeProto(const std::string& name,
                std::unique_ptr<parser::Prototype> proto) {
  gFuncProtos_[name] = std::move(proto);
}

llvm::Function* getFunction(const std::string name) {
  auto f = gModule_->getFunction(name);
  if (f != nullptr) {
    return f;
  }

  auto fi = gFuncProtos_.find(name);
  if (fi != gFuncProtos_.end()) {
    return fi->second->codeGen();
  }

  return nullptr;
}

int getBinOpTokPrecedence(lexer::Token tok) {
  auto it = binOpPrec.find(tok);
  if (it == binOpPrec.end() || it->second <= 0) {
    return -1;
  } else {
    return it->second;
  }
}

void setBinOpTokPrecedence(lexer::Token tok, int prec) {
  binOpPrec[tok] = prec;
}

void eraseBinOpTok(lexer::Token tok) { binOpPrec.erase(tok); }

}  // namespace global

}  // namespace kaso
