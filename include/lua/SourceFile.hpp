#pragma once

#include <vector>
#include <CTRPluginFramework/System/File.hpp>

#include "Token.hpp"
#include "ASTFwd.hpp"
#include "Errors.hpp"

namespace CTRPluginFramework::lua {

class Lexer;
class Parser;

struct SourceFile {
  std::string path;
  std::string data;

  std::vector<SourceFile*> imports;

  std::vector<Error*> errors;

  File file;

  Lexer* lexer;
  Parser* parser;

  Token* token;
  ast::Program* program;

  auto add_error(Error const& e) -> Error& {
    return *this->errors.emplace_back(new Error(e));
  }

  auto read() -> bool;

  auto get_line_range(size_t pos) -> std::pair<size_t, size_t>;

  auto length() -> size_t const { return this->data.length(); }

  SourceFile(std::string const& path);
  ~SourceFile();
};

}  // namespace CTRPluginFramework::lua