#include "shell.h"
#include <iostream>

/// putchard - putchar that takes a double and returns 0.
extern "C" double putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

namespace kaso {
namespace shell {

Shell::Shell() {
  lexer::Lexer lex(std::cin);
  myParser_ = std::make_unique<parser::Parser>(lex);
}

void Shell::repl(bool verbose) {
  global::init();
  global::initModuleAndPassManager();

  fprintf(stderr, "ready> ");
  myParser_->getNextToken();
  while (true) {
    if (myParser_->curToken() == lexer::Token::Eof) {
      break;
    }
    switch (myParser_->curToken()) {
      case lexer::Token::Semicolon:
        fprintf(stderr, "ready> ");
        myParser_->getNextToken();
        break;
      case lexer::Token::Def:
        handleDefinition(verbose);
        break;
      case lexer::Token::Extern:
        handleExtern(verbose);
        break;
      default:
        handleTopLevelExpression(verbose);
        break;
    }
  }

  if (verbose) {
    global::gModule()->print(llvm::errs(), nullptr);
  }
}

void Shell::handleDefinition(bool verbose) {
  auto fn = myParser_->definition();
  if (fn != nullptr) {
    if (auto fnIR = fn->codeGen()) {
      if (verbose) {
        fprintf(stderr, "Read function definition: ");
        fnIR->print(llvm::errs());
        fprintf(stderr, "\n");
      }
      global::gJIT()->addModule(std::move(global::gModule()));
      global::initModuleAndPassManager();
    }
  } else {
    myParser_->getNextToken();
  }
}

void Shell::handleExtern(bool verbose) {
  if (auto proto = myParser_->externDef()) {
    if (auto fnIR = proto->codeGen()) {
      if (verbose) {
        fprintf(stderr, "Read extern: ");
        fnIR->print(llvm::errs());
        fprintf(stderr, "\n");
      }
      auto name = proto->getName();
      global::storeProto(name, std::move(proto));
    }
  } else {
    myParser_->getNextToken();
  }
}

void Shell::handleTopLevelExpression(bool verbose) {
  if (auto fn = myParser_->topLevelExpr()) {
    if (auto fnIR = fn->codeGen()) {
      if (verbose) {
        fprintf(stderr, "Read top-level expression:\n");
        fnIR->print(llvm::errs());
        fprintf(stderr, "\n");
      }

      auto handle = global::gJIT()->addModule(std::move(global::gModule()));
      global::initModuleAndPassManager();

      auto exprSymbol = global::gJIT()->findSymbol("__anonymous_expr");
      assert(exprSymbol && "Function not found");

      auto addr = exprSymbol.getAddress();
      using FP = double (*)();
      auto fp = (FP)(intptr_t)llvm::cantFail(std::move(addr));
      auto val = fp();
      if (verbose) {
        fprintf(stderr, "Evaluated to %f\n", val);
      }
      global::gJIT()->removeModule(handle);
    }
  } else {
    myParser_->getNextToken();
  }
}

}  // namespace shell
}  // namespace kaso
