#include "lua/Token.hpp"
#include "lua/Lexer.hpp"

namespace CTRPluginFramework::lua {

Token Token::empty;

Lexer* Token::get_lexer() { return this->source->lexer; }

std::string_view Token::get_strview() {
  if (this->is(TokenKind::Keyword)) return Lexer::get_view_of(this->kwd);

  if (this->is(TokenKind::Punctuator)) {
    if (this->punct != TokPunctuators::_) {
      if (this->is(TokPunctuators::Bracket))
        return Lexer::get_view_of(this->bracket, this->is_brac_open);
      return Lexer::get_view_of(this->punct);
    }
    else if (this->op != TokOperators::_)
      return Lexer::get_view_of(this->op);
  }

#ifdef _LUA_DEBUG_
  if (this->is(TokenKind::Unknown)) return "<@Unknown>";
  if (this->is(TokenKind::Eof)) return "<@Eof>";
#endif

  return this->get_lexer()->get_strview(this->str);
}

std::string_view Token::get_line_view(int offs, int count) {
  while (true);

  return "tinko";
}

StringID Token::set_string(std::string const& str) {
  return (this->str = this->get_lexer()->get_str_id(str));
}

}  // namespace CTRPluginFramework::lua