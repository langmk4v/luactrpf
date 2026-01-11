#pragma once

#include "types.hpp"

namespace CTRPluginFramework::lua {

enum class TypeKind : u8 {
  None,
  I32,
  U32,
  Float,
  Bool,
  Str,
};

struct TypeInfo {
  TypeKind kind;

  TypeInfo(TypeKind kind = TypeKind::None)
    : kind(kind)
  {
  }
};

}