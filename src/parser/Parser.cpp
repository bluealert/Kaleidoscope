#include "parser/Parser.h"

namespace kaso {
namespace parser {

Parser::Parser(lexer::Lexer lex)
    : lexer_(std::move(lex)), curTok_(lexer::Token::Error) {}

std::unique_ptr<Expr> Parser::numberExpr() {
  auto result = std::make_unique<NumberExpr>(lexer_.numVal());
  getNextToken();
  return std::move(result);
}

std::unique_ptr<Expr> Parser::parenExpr() {
  getNextToken();
  auto v = expression();
  if (!v) {
    return nullptr;
  }
  if (curTok_ != lexer::Token::RightParen) {
    return logError("expected ')'");
  }
  getNextToken();
  return v;
}

std::unique_ptr<Expr> Parser::identifierExpr() {
  std::string idName = lexer_.strVal();

  getNextToken();  // eat identifier
  if (curTok_ != lexer::Token::LeftParen) {
    return std::make_unique<VariableExpr>(idName);
  }

  getNextToken();  // eat '('
  std::vector<std::unique_ptr<Expr>> args;
  if (curTok_ != lexer::Token::RightParen) {
    while (true) {
      auto arg = expression();
      if (arg != nullptr) {
        args.push_back(std::move(arg));
      } else {
        return nullptr;
      }

      if (curTok_ == lexer::Token::RightParen) {
        break;
      }

      if (curTok_ != lexer::Token::Comma) {
        return logError("Expected ')' or ',' in argument list");
      }

      getNextToken();
    }
  }

  getNextToken();  // eat ')'
  return std::make_unique<CallExpr>(idName, std::move(args));
}

std::unique_ptr<Expr> Parser::primary() {
  switch (curTok_) {
    case lexer::Token::Identifier:
      return identifierExpr();
    case lexer::Token::Number:
      return numberExpr();
    case lexer::Token::LeftParen:
      return parenExpr();
    case lexer::Token::If:
      return ifExpr();
    case lexer::Token::For:
      return forExpr();
    default:
      return logError("unknown token when expecting an expression");
  }
}

std::unique_ptr<Expr> Parser::expression() {
  auto lhs = unary();
  if (!lhs) {
    return nullptr;
  }

  return binOpRHS(0, std::move(lhs));
}

// the minimal operator precedence that the function is allowed to eat.
std::unique_ptr<Expr> Parser::binOpRHS(int prec, std::unique_ptr<Expr> lhs) {
  while (true) {
    auto tokPrec = global::getBinOpTokPrecedence(curTok_);
    if (tokPrec < prec) {
      return lhs;
    }

    // ok, we know this is a bin op.
    auto binOp = curTok_;
    auto binOpStr = lexer_.strVal();
    getNextToken();

    // parse the unary expression after the binary operator
    auto rhs = unary();
    if (!rhs) {
      return nullptr;
    }

    auto nextTokPrec = global::getBinOpTokPrecedence(curTok_);
    if (nextTokPrec > tokPrec) {
      rhs = binOpRHS(tokPrec + 1, std::move(rhs));
      if (!rhs) {
        return nullptr;
      }
    }

    lhs = std::make_unique<BinaryExpr>(binOp, binOpStr, std::move(lhs),
                                       std::move(rhs));
  }
}

std::unique_ptr<Prototype> Parser::prototype() {
  std::string fnName;
  uint32_t kind = 0;  // 0 = identifier, 1 = unary, 2 = binary.
  uint32_t binaryPrecedence = 30;

  auto op = lexer::Token::Error;
  switch (curTok_) {
    case lexer::Token::Identifier:
      fnName = lexer_.strVal();
      kind = 0;
      getNextToken();
      break;

    case lexer::Token::Unary:
      getNextToken();
      if (!lexer::isValidUnaryOperator(curTok_)) {
        return logErrorP("Invalid unary operator");
      }

      fnName = "unary";
      fnName += lexer_.strVal();
      kind = 1;
      op = curTok_;

      getNextToken();
      break;

    case lexer::Token::Binary: {
      getNextToken();
      if (!lexer::isValidBinaryOperator(curTok_)) {
        return logErrorP("Invalid binary operator");
      }

      fnName = "binary";
      fnName += lexer_.strVal();
      kind = 2;
      op = curTok_;

      getNextToken();
      if (curTok_ == lexer::Token::Number) {
        if (lexer_.numVal() < 1 || lexer_.numVal() > 100) {
          return logErrorP("Invalid precedence: must be 1..100");
        }
        binaryPrecedence = (uint32_t)lexer_.numVal();
        getNextToken();
      }
      break;
    }
    default:
      return logErrorP("Expected function name in prototype");
  }

  if (curTok_ != lexer::Token::LeftParen) {
    return logErrorP("Expected '(' in prototype");
  }

  std::vector<std::string> argNames;
  while (getNextToken() == lexer::Token::Identifier) {
    argNames.push_back(lexer_.strVal());
  }
  if (curTok_ != lexer::Token::RightParen) {
    return logErrorP("Expected ')' in prototype");
  }

  getNextToken();

  if (kind && argNames.size() != kind) {
    return logErrorP("Invalid number of operands for operator");
  }

  return std::make_unique<Prototype>(fnName, std::move(argNames), kind != 0, op,
                                     binaryPrecedence);
}

std::unique_ptr<Function> Parser::definition() {
  getNextToken();
  auto proto = prototype();
  if (!proto) {
    return nullptr;
  }
  if (auto e = expression()) {
    return std::make_unique<Function>(std::move(proto), std::move(e));
  }
  return nullptr;
}

std::unique_ptr<Function> Parser::topLevelExpr() {
  if (auto e = expression()) {
    auto proto = std::make_unique<Prototype>("__anonymous_expr",
                                             std::vector<std::string>());
    return std::make_unique<Function>(std::move(proto), std::move(e));
  }
  return nullptr;
}

std::unique_ptr<Prototype> Parser::externDef() {
  getNextToken();
  return prototype();
}

std::unique_ptr<Expr> Parser::ifExpr() {
  getNextToken();
  auto cond = expression();
  if (cond == nullptr) {
    return nullptr;
  }

  if (curTok_ != lexer::Token::Then) {
    return logError("expected then");
  }

  getNextToken();
  auto then = expression();
  if (then == nullptr) {
    return nullptr;
  }

  if (curTok_ != lexer::Token::Else) {
    return logError("expected else");
  }

  getNextToken();
  auto els = expression();
  if (els == nullptr) {
    return nullptr;
  }

  return std::make_unique<IfExpr>(std::move(cond), std::move(then),
                                  std::move(els));
}

std::unique_ptr<Expr> Parser::forExpr() {
  getNextToken();

  if (curTok_ != lexer::Token::Identifier) {
    return logError("expected identifier after for");
  }

  std::string idName = lexer_.strVal();
  getNextToken();

  if (curTok_ != lexer::Token::OpAssign) {
    return logError("expected '=' after for");
  }
  getNextToken();

  auto start = expression();
  if (start == nullptr) {
    return nullptr;
  }
  if (curTok_ != lexer::Token::Comma) {
    return logError("expected ',' after for start value");
  }
  getNextToken();

  auto end = expression();
  if (end == nullptr) {
    return nullptr;
  }

  std::unique_ptr<Expr> step;
  if (curTok_ == lexer::Token::Comma) {
    getNextToken();
    step = expression();
    if (step == nullptr) {
      return nullptr;
    }
  }

  if (curTok_ != lexer::Token::In) {
    return logError("expected 'in' after for");
  }
  getNextToken();

  auto body = expression();
  if (body == nullptr) {
    return nullptr;
  }

  return std::make_unique<ForExpr>(idName, std::move(start), std::move(end),
                                   std::move(step), std::move(body));
}

std::unique_ptr<Expr> Parser::unary() {
  if (!lexer::isValidUnaryOperator(curTok_)) {
    return primary();
  }

  auto op = curTok_;
  auto opStr = lexer_.strVal();
  getNextToken();
  if (auto operand = unary()) {
    return std::make_unique<UnaryExpr>(op, opStr, std::move(operand));
  }

  return nullptr;
}

lexer::Token Parser::getNextToken() {
  curTok_ = lexer_.getTok();
  return curTok_;
}

lexer::Token Parser::curToken() { return curTok_; }

}  // namespace parser
}  // namespace kaso
