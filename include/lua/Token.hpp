#pragma once

#include "types.hpp"

namespace CTRPluginFramework::lua {

using StringID = size_t;

struct SourceFile;
class Lexer;

enum class TokenKind {
  Unknown,
  Literal,
  Identifier,
  Keyword,
  Punctuator,
  Eof,
};

enum class TokenLiterals {
  _,

  I32,  // `L?
  I64,  // `LL
  U32,  // hex or bin or `U
  U64,  // hex or bin or `UL

  Double, // (f or F)?

  String,
  StringEmpty,
};

enum class TokOperators {
  _,

  // 文字数長い順に定義！！！

  LShiftAssign,
  RShiftAssign,
  AddAssign,
  SubAssign,
  MulAssign,
  DivAssign,
  ModAssign,
  BitAndAssign,
  BitOrAssign,
  BitXorAssign,
  LShift,    //  <<
  RShift,    //  >>
  Equal,     //  ==
  NotEqual,  //  !=
  GreaterOrEq,
  LessOrEq,
  LogAnd,
  LogOr,
  Assign,
  Greater,
  Less,
  BitAnd,
  BitOr,
  BitXor,
  BitNot,
  LogNot,
  Add,  //  +
  Sub,  //  -
  Mul,  //  *
  Div,  //  /
  Mod,  //  %
};

enum class TokPunctuators {
  _,
  Bracket,  // TokBrackets

  // 文字数長い順に定義！！！

  Ellipsis,    // ...
  RightArrow,  // ->
  Doller,      // $
  Sharp,       // #
  Colon,       // :
  Semi,        // ;
  Dot,         // .
  Comma,       // ,
  Atmark,      // @
};

enum class TokBrackets {
  _,
  Normal,  // ( )
  Scope,   // { }
  Array,   // [ ]
  Angle,   // < >
};

enum class TokKeywords {
  _,

  True,
  False,

  None,
  I32,
  F32,

  And,
  Or,

  If,
  Then,
  Else,
  Elseif,

  For,
  While,

  Repeat,
  Until,

  Do,
  End,

  Break,
  Return,

  Fn,
  Enum,

  Import,
};

struct Token {
  TokenKind kind = TokenKind::Unknown;
  TokenLiterals literal = TokenLiterals::_;

  TokOperators op = TokOperators::_;
  TokPunctuators punct = TokPunctuators::_;
  TokKeywords kwd = TokKeywords::_;

  TokBrackets bracket = TokBrackets::_;
  bool is_brac_open = false;

  Token* prev = nullptr;
  Token* next = nullptr;

  StringID str = 0;
  size_t length = 0;

  size_t position = 0;
  size_t line = 0;
  size_t column = 0;

  SourceFile* source = nullptr;

  union {
    i32 v_i32;
    i64 v_i64;
    u32 v_u32;
    u64 v_u64 = 0;

    double v_dbl;

    u32 _v_char_full;
    struct {
      char16_t v_char;
      char16_t v_char_surrogate;
    };

    std::u16string* v_str;  // string literal
                            // (object has hold by Lexer class.)
  };

  static Token empty;

  Token(TokenKind kind = TokenKind::Unknown) : kind(kind) {}

  Token(TokenKind kind, Token* prev, SourceFile* source)
      : kind(kind), prev(prev), source(source) {
    if (prev) prev->next = this;
  }

  Token(Token const&) = delete;
  Token(Token&&) = delete;

  ~Token() {
    if (this->next) delete this->next;
  }

  Lexer* get_lexer();

  string_view get_strview();

  string_view get_line_view(int offs = 0, int count = 1);

  StringID set_string(string const&);

  bool is(TokenKind k) const { return this->kind == k; }

  bool is(TokOperators k) const {
    return is(TokenKind::Punctuator) && this->op == k;
  }

  bool is(TokPunctuators k) const {
    return is(TokenKind::Punctuator) && this->punct == k;
  }

  bool is(TokBrackets k, bool is_open) const {
    return is(TokPunctuators::Bracket)
          && this->bracket == k
          && this->is_brac_open == is_open;
  }

  bool is(TokKeywords k) const {
    return is(TokenKind::Keyword) && this->kwd == k;
  }

  bool is_semi() const { return is(TokPunctuators::Semi); }

  bool is_ident() const { return is(TokenKind::Identifier); }
  bool is_punct() const { return is(TokenKind::Punctuator); }
  bool is_op() const { return is_punct() && punct == TokPunctuators::_; }
  bool is_kwd() const { return is(TokenKind::Keyword); }

  bool is_eof() const { return is(TokenKind::Eof); }
};

}  // namespace CTRPluginFramework::lua