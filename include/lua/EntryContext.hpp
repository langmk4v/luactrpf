#pragma once

#include <CTRPluginFramework/Menu.hpp>

#include "SourceFile.hpp"
#include "Parser.hpp"
#include "Eval.hpp"
#include "Logger.hpp"

namespace CTRPluginFramework::lua {

struct EntryContext {
  SourceFile source;
  ASTEvaluator evaluator;

  auto is_parsed() -> bool {
    return source.program != nullptr;
  }

  auto get_token() -> Token*
    { return this->source.token; }

  auto lex() -> void {
    this->source.token = this->source.lexer->lex();
  }

  auto parse() -> void {
    this->source.program = this->source.parser->parse(this->get_token());
  }

  auto eval() -> void {
    this->evaluator.eval(this->source.program);
  }

  EntryContext(std::string const& path, MenuEntry* e)
    : source(path),
      evaluator(&source, e)
  {
  }
};

static auto add_entry(PluginMenu& menu, std::string const& path, MenuEntry* e) -> MenuEntry* {

  auto* ctx = new EntryContext(path, e);

  if (!ctx->source.read()) {
    OSD::Notify("failed to read '" + ctx->source.path + "'");
    goto Fail;
  }

  ctx->lex();
  if (!ctx->get_token()) {
    OSD::Notify(path + ": failed to tokenize.");
    goto Fail;
  }

  ctx->parse();
  if (!ctx->source.program) {
    OSD::Notify(path + ": failed to parse.");
    goto Fail;
  }

  e->SetArg(ctx);

  menu.Append(e);

  return e;

Fail:
  for(auto&&e:ctx->source.errors)
    Logger::Emit(e->get_emit_message());

  delete ctx;
  return nullptr;
}

}