#include "CTRPluginFramework.hpp"

#include "lua.hpp"
#include "lua/Logger.hpp"

namespace CTRPluginFramework {

auto entry_test(MenuEntry* e) -> void {

  auto ctx = static_cast<lua::EntryContext*>(e->GetArg());

  if (!ctx)
    return;

  ctx->eval();

  if (Controller::IsKeyPressed(Key::X)) {

    std::string msg;

    for (auto tok = ctx->get_token(); !tok->is_eof(); tok = tok->next) {
      msg += tok->get_strview();
      msg += " ";
    }

    (MessageBox(msg))();
  }

  if (Controller::IsKeyPressed(Key::L)) {
    OSD::Notify(Utils::Format("%08X", (u32)ctx));
  }

}

auto init_menu(PluginMenu& menu) -> void {

  lua::add_entry(menu, "test.zlua", new MenuEntry("test", entry_test));

}

auto test() -> void {
  auto src = lua::SourceFile("test.zlua");

  auto lexer = src.lexer;

  auto token = lexer->lex();

  delete token;
}

auto main() -> int {
  File  logfile("compiling.log", File::CREATE | File::WRITE);

  lua::Logger::SetFile(&logfile);
  lua::Logger::Emit("----------------------");
  lua::Logger::Flush();

  PluginMenu menu{"CTRPF"};

  menu.SynchronizeWithFrame(true);

  init_menu(menu);

  menu.Run();

  return 0;
}

}  // namespace CTRPluginFramework