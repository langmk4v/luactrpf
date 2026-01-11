#pragma once

#include <memory>

#include <CTRPluginFramework/System.hpp>
#include <CTRPluginFramework/Utils.hpp>

#define  alert  (Logger::Emit(Utils::Format("#alert: " __FILE__ ":%d", __LINE__)))

namespace CTRPluginFramework::lua {

class Logger {
  static inline std::unique_ptr<LineWriter> writer;
  static inline File* fp = nullptr;

public:
  static auto SetFile(File* F) -> void {
    fp = F;
    writer = std::make_unique<LineWriter>(*fp);
  }

  static auto GetFile() -> File& {
    return *fp;
  }

  static auto GetPath() -> std::string {
    return fp->GetName();
  }

  static auto Emit(std::string const& text) -> void {
    *writer << text << LineWriter::endl();
    Flush();
  }

  static auto Flush() -> void {
    writer->Flush();
  }
};

} // namespace CTRPluginFramework::lua