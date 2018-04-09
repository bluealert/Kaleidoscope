#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include "KaleidoscopeJIT.h"
#include "parser/Expr.h"
#include "parser/Function.h"

namespace kaso {
namespace global {

void init();

void initModuleAndPassManager();

llvm::LLVMContext& gContext();

llvm::IRBuilder<>& gBuilder();

std::unique_ptr<llvm::Module>& gModule();

std::map<std::string, llvm::Value*>& gNamedValues();

std::unique_ptr<llvm::legacy::FunctionPassManager>& gFPM();

std::unique_ptr<llvm::orc::KaleidoscopeJIT>& gJIT();

void storeProto(const std::string& name,
                std::unique_ptr<parser::Prototype> proto);

llvm::Function* getFunction(std::string name);

int getBinOpTokPrecedence(lexer::Token tok);
void setBinOpTokPrecedence(lexer::Token tok, int prec);
void eraseBinOpTok(lexer::Token tok);

}  // namespace global

std::unique_ptr<parser::Expr> logError(const char* s);

std::unique_ptr<parser::Prototype> logErrorP(const char* s);

llvm::Value* logErrorV(const char* s);

}  // namespace kaso
