#pragma once

#include <string>
#include <vector>

#include "Token.hpp"
#include "SourceFile.hpp"

namespace CTRPluginFramework::lua {

class Lexer {

  template <typename K>
  using kind_str_map_t = std::pair<K, char const*>;

  static inline const kind_str_map_t<TokOperators> g_op_str_table[] {
    { TokOperators::_, nullptr },
    { TokOperators::LShiftAssign, "<<=" },
    { TokOperators::RShiftAssign, ">>=" },
    { TokOperators::AddAssign,    "+=" },
    { TokOperators::SubAssign,    "-=" },
    { TokOperators::MulAssign,    "*=" },
    { TokOperators::DivAssign,    "/=" },
    { TokOperators::ModAssign,    "%=" },
    { TokOperators::BitAndAssign, "&=" },
    { TokOperators::BitOrAssign,  "|=" },
    { TokOperators::BitXorAssign, "^=" },
    { TokOperators::LShift,       "<<" },
    { TokOperators::RShift,       ">>" },
    { TokOperators::Equal,        "==" },
    { TokOperators::NotEqual,     "!=" },
    { TokOperators::GreaterOrEq,  ">=" },
    { TokOperators::LessOrEq,     "<=" },
    { TokOperators::LogAnd,       "&&" },
    { TokOperators::LogOr,        "||" },
    { TokOperators::Assign,       "=" },
    { TokOperators::Greater,      ">" },
    { TokOperators::Less,         "<" },
    { TokOperators::BitAnd,       "&" },
    { TokOperators::BitOr,        "|" },
    { TokOperators::BitXor,       "^" },
    { TokOperators::BitNot,       "~" },
    { TokOperators::LogNot,       "!" },
    { TokOperators::Add,          "+" },
    { TokOperators::Sub,          "-" },
    { TokOperators::Mul,          "*" },
    { TokOperators::Div,          "/" },
    { TokOperators::Mod,          "%" },
  };

  static inline const kind_str_map_t<TokPunctuators> g_punct_str_table[] {
    { TokPunctuators::_,          nullptr },
    { TokPunctuators::Bracket,    nullptr },
    { TokPunctuators::Ellipsis,   "..." },
    { TokPunctuators::RightArrow, "->" },
    { TokPunctuators::Doller,     "$" },
    { TokPunctuators::Sharp,      "#" },
    { TokPunctuators::Colon,      ":" },
    { TokPunctuators::Semi,       ";" },
    { TokPunctuators::Dot,        "." },
    { TokPunctuators::Comma,      "," },
    { TokPunctuators::Atmark,     "@" },
  };

  static inline const kind_str_map_t<TokBrackets> g_bracket_str_table[] {
    { TokBrackets::_,       nullptr },
    { TokBrackets::Normal,  "()" },
    { TokBrackets::Scope,   "{}" },
    { TokBrackets::Array,   "[]" },
    { TokBrackets::Angle,   "<>" },
  };

  static inline const kind_str_map_t<TokKeywords> g_kwd_str_table[] {
    { TokKeywords::_,         nullptr },

    { TokKeywords::True,      "true"  },
    { TokKeywords::False,     "false" },

    { TokKeywords::None,      "None"      },
    { TokKeywords::I32,       "i32"       },
    { TokKeywords::F32,       "f32"       },

    { TokKeywords::And,       "and"       },
    { TokKeywords::Or,        "or"        },

    { TokKeywords::If,        "if"        },
    { TokKeywords::Then,      "then"      },
    { TokKeywords::Else,      "else"      },
    { TokKeywords::Elseif,    "elseif"    },

    { TokKeywords::For,       "for"       },
    { TokKeywords::While,     "while"     },

    { TokKeywords::Repeat,    "repeat"    },
    { TokKeywords::Until,     "until"     },

    { TokKeywords::Do,        "do"        },
    { TokKeywords::End,       "end"       },

    { TokKeywords::Break,     "break"     },
    { TokKeywords::Return,    "return"    },

    { TokKeywords::Fn,        "function"  },
    { TokKeywords::Enum,      "enum"      },
    { TokKeywords::Import,    "import"    },
  };

  SourceFile* source;

  std::vector<string> strpool;

  std::vector<std::u16string> str_literal_pool;

  std::vector<std::pair<Token*, string_view>> str_literal_create_task;

  size_t position = 0;

  size_t line = 1;
  size_t column = 1;

  void enter() {
    line++;
    column = 1;
  }
  bool chk_enter() { return match('\n') ? (enter(), 1) : 0; }

  void next(int count = 1) {
    while (count--) {
      chk_enter() || this->column++;
      this->position++;
    }
  }

  auto is_end() -> bool
    { return this->position >= this->source->data.length(); }

  auto peek() -> char
    { return this->source->data[this->position]; }

  auto pass_space() -> void
    { while (!this->is_end() && std::isspace(this->peek())) this->next(); }

  void pass_comments();
  bool pass_comment();
  bool pass_block_comment();

  bool match(char);

  bool match_str(string_view);

  bool consume(char);

  bool consume_str(string_view);

  char* cur() { return this->source->data.data() + this->position; }

  void create_str_literals();

 public:
  Lexer(SourceFile* source) : source(source), position(0) {}

  ~Lexer() {}

  auto lex() -> Token*;

  auto get_strview(StringID) -> string_view;

  auto get_str_id(string_view) -> StringID;

  auto get_str_id(char const* ptr, size_t len) -> StringID;

  static auto get_view_of(TokOperators) -> string_view;
  static auto get_view_of(TokPunctuators) -> string_view;
  static auto get_view_of(TokBrackets, bool) -> string_view;
  static auto get_view_of(TokKeywords) -> string_view;
};

}  // namespace CTRPluginFramework::lua