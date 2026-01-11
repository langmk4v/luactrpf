#pragma once

#include <string>

#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"

#include "Token.hpp"

namespace CTRPluginFramework::lua {

struct Error {
 public:
  size_t line;
  size_t column;
  std::string msg;

  Error(size_t line,size_t column, std::string const& msg)
    : line(line),column(column),msg(msg) {}

  Error(Token* token, std::string const& msg)
    : line(token->line),column(token->column),msg(msg) {}

  std::string get_emit_message() {
    return Utils::Format("error at line=%zu, column=%zu: ",line,column) + msg;
  }
};

}  // namespace CTRPluginFramework::lua