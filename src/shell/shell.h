#pragma once

#include "parser/Parser.h"

namespace kaso {
namespace shell {

class Shell {
 public:
  Shell();

  /// top ::= definition | external | expression | ';'
  void repl(bool verbose);

 private:
  void handleDefinition(bool verbose);
  void handleExtern(bool verbose);
  void handleTopLevelExpression(bool verbose);

 private:
  std::unique_ptr<parser::Parser> myParser_;
};

}  // namespace shell
}  // namespace kaso
