#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"

namespace kaso {
namespace parser {

TEST(ParserTest, DefinitionTest1) {
  std::stringstream ss;
  ss << "def foo(x y) x+foo(y, 4.0);" << std::endl;
  lexer::Lexer lex(ss);
  Parser par(lex);

  ASSERT_EQ(lex.getTok(), lexer::Token::Def);
  ASSERT_EQ(lex.strVal(), "def");
  ASSERT_NE(par.definition(), nullptr);
}

TEST(ParserTest, DefinitionTest2) {
  std::stringstream ss;
  ss << "def foo(x y) x+y );" << std::endl;
  lexer::Lexer lex(ss);
  Parser par(lex);

  ASSERT_EQ(lex.getTok(), lexer::Token::Def);
  ASSERT_EQ(lex.strVal(), "def");
  ASSERT_NE(par.definition(), nullptr);
  ASSERT_EQ(par.topLevelExpr(), nullptr);
}

TEST(ParserTest, ExternTest) {
  std::stringstream ss;
  ss << "extern sin(a);" << std::endl;
  lexer::Lexer lex(ss);
  Parser par(lex);

  ASSERT_EQ(lex.getTok(), lexer::Token::Extern);
  ASSERT_EQ(lex.strVal(), "extern");
  ASSERT_NE(par.externDef(), nullptr);
  ASSERT_EQ(lex.getTok(), lexer::Token::Eof);
}

}  // namespace parser
}  // namespace kaso