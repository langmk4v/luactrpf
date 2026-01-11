#include <cctype>
#include <cstring>
#include <cassert>

#include "lua.hpp"
#include "lua/utf.hpp"

#define todo (std::abort())

namespace CTRPluginFramework::lua {

string_view Lexer::get_view_of(TokOperators k) {
  return g_op_str_table[static_cast<size_t>(k)].second;
}

string_view Lexer::get_view_of(TokPunctuators k) {
  return g_punct_str_table[static_cast<size_t>(k)].second;
}

string_view Lexer::get_view_of(TokBrackets k, bool b) {
  return string_view(
      g_bracket_str_table[static_cast<size_t>(k)].second + static_cast<int>(b),
      1);
}

string_view Lexer::get_view_of(TokKeywords k) {
  return g_kwd_str_table[static_cast<size_t>(k)].second;
}

auto Lexer::lex() -> Token* {
  Token head;
  Token* cur = &head;

  this->pass_space();
  this->pass_comments();

  while (!this->is_end()) {
    char c = this->peek();
    char* ptr = this->cur();
    size_t begin = this->position;

    size_t line_ = this->line;
    size_t column_ = this->column;

    // hex
    if (this->consume_str("0x") || this->consume_str("0X")) {
      cur = new Token(TokenKind::Literal, cur, this->source);

      while (!this->is_end() && std::isxdigit(this->peek()))
        this->next();

      cur->literal = TokenLiterals::U32;

      cur->v_u32 =
          std::stoi(std::string(ptr, (this->position - begin)), nullptr, 16);
    }

    // bin
    else if (this->consume_str("0b") || this->consume_str("0B")) {
      cur = new Token(TokenKind::Literal, cur, this->source);

      while (!this->is_end() && std::isxdigit(this->peek()))
        this->next();

      cur->literal = TokenLiterals::U32;

      cur->v_u32 =
          std::stoi(std::string(ptr, (this->position - begin)), nullptr, 2);
    }

    ///
    /// digits
    else if (std::isdigit(c)) {
      cur = new Token(TokenKind::Literal, cur, this->source);

      while (!this->is_end() && std::isdigit(this->peek()))
        this->next();

      ///
      /// Double
      if (this->consume('.')) {
        while (!this->is_end() && std::isdigit(this->peek()))
          this->next();

        this->consume('f') || this->consume('F');

        cur->literal = TokenLiterals::Float;
        cur->v_float = (float)std::atof(ptr);
      }

      //
      // Check suffixes
      //
      else if (this->consume('U'))
        cur->literal = TokenLiterals::U32;
      else if (this->consume_str("UL"))
        cur->literal = TokenLiterals::U64;
      else if (this->consume_str("LL"))
        cur->literal = TokenLiterals::I64;
      else {
        cur->literal = TokenLiterals::I32;
        this->consume('l');
      }
    }

    //
    // identifier
    else if (std::isalpha(c) || c == '_') {
      cur = new Token(TokenKind::Identifier, cur, this->source);

      while (!this->is_end() && (std::isalnum((c = this->peek())) || c == '_'))
        this->next();

      for (auto&& [_k, _s] : g_kwd_str_table)
        if (_s && (this->position - begin) == std::strlen(_s) &&
            std::strncmp(ptr, _s, (this->position - begin)) == 0) {
          cur->kind = TokenKind::Keyword;
          cur->kwd = _k;
          break;
        }
    }

    //
    // string literal
    else if (this->consume('"') || this->consume('\'')) {
      if (this->consume(c)) {
        cur = new Token(TokenKind::Literal, cur, this->source);
        cur->literal = TokenLiterals::StringEmpty;
      } else {
        while (!this->consume(c)) {
          if (this->is_end()) {
            this->source->add_error(Error(line_,column_, "not closed string literal."));
            break;
          }

          this->next();
        }

        cur = new Token(TokenKind::Literal, cur, this->source);
        cur->literal = TokenLiterals::String;

        assert((this->cur() - ptr) >= 2);

        this->str_literal_create_task.emplace_back(
            cur, string_view(ptr + 1, this->position - begin - 2));
      }
    }

    //
    // other
    else {
      cur = new Token(TokenKind::Punctuator, cur, this->source);

      for (auto&& [K, S] : g_punct_str_table) {
        if (S && this->consume_str(S)) {
          cur->punct = K;
          goto _end_of_find;
        }
      }

      for (auto&& [K, S] : g_op_str_table) {
        if (S && this->consume_str(S)) {
          cur->op = K;
          goto _end_of_find;
        }
      }

      for (auto&& [K, S] : g_bracket_str_table) {
        if (S && this->consume(S[0])) {
          cur->punct = TokPunctuators::Bracket;
          cur->bracket = K;
          cur->is_brac_open = true;
          goto _end_of_find;
        } else if (S && this->consume(S[1])) {
          cur->punct = TokPunctuators::Bracket;
          cur->bracket = K;
          cur->is_brac_open = false;
          goto _end_of_find;
        }
      }

      this->source->add_error(Error(line_,column_,"invalid token"));

    _end_of_find:;
    }

    cur->position = begin;
    cur->length = this->position - cur->position;
    cur->str = this->get_str_id(ptr, cur->length);
    cur->line = line_;
    cur->column = column_;

    this->pass_space();
    this->pass_comments();
  }

  cur = new Token(TokenKind::Eof, cur, this->source);

  this->create_str_literals();

  Token* root = head.next;

  head.next = nullptr;

  return root;
}

void Lexer::create_str_literals() {
  this->str_literal_pool.reserve(this->str_literal_create_task.size());

  for (auto& [tok, view] : this->str_literal_create_task) {
    std::u16string& s =
        this->str_literal_pool.emplace_back(utf::utf8_to_utf16(view));

    tok->v_str = &s;
  }

  this->str_literal_create_task.clear();
}

auto Lexer::get_strview(StringID id) -> string_view {
  return this->strpool[id];
}

auto Lexer::get_str_id(string_view s) -> StringID {
  for (StringID i = 0; i < this->strpool.size(); i++)
    if (this->strpool[i].length() == s.length() && this->strpool[i] == s)
      return i;

  this->strpool.push_back(string(s));

  return this->strpool.size() - 1;
}

auto Lexer::get_str_id(char const* ptr, size_t len) -> StringID {
  for (StringID i = 0; i < this->strpool.size(); i++)
    if (this->strpool[i].length() == len &&
        std::strncmp(this->strpool[i].data(), ptr, len) == 0)
      return i;

  this->strpool.push_back(string(ptr, len));

  return this->strpool.size() - 1;
}

void Lexer::pass_comments() {
  while (!this->is_end()) {
    this->pass_space();
    if (!this->pass_comment()) break;
  }
}

bool Lexer::pass_comment() {
  if (this->consume_str("--")) {
    while (!this->is_end() && !this->consume('\n')) {
      if (this->match_str("[[")) {
        this->pass_block_comment();
        break;
      }

      this->next();
    }
    return true;
  } else
    return false;
}

bool Lexer::pass_block_comment() {
  if (this->consume_str("[[")) {
    while (!this->is_end() && !this->consume_str("]]")) this->next();
    return true;
  } else
    return false;
}

bool Lexer::match(char c) { return this->peek() == c; }

bool Lexer::match_str(string_view str) {
  return std::strncmp(this->cur(), str.data(), str.length()) == 0;
}

bool Lexer::consume(char c) {
  if (match(c)) {
    next();
    return true;
  } else
    return false;
}

bool Lexer::consume_str(string_view str) {
  return this->match_str(str) && (next(str.length()), true);
}

} // namespace CTRPluginFramework::lua