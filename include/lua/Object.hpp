#pragma once

#include "TypeInfo.hpp"

namespace CTRPluginFramework::lua {

struct __attribute__((__packed__)) Object {

  TypeInfo type;

  union {
    i32 v_i32 = 0;
    u32 v_u32;
    float v_float;
    bool v_bool;
    std::u16string *v_str;
  };

  Object(TypeInfo type) : type(type) {}

  ~Object() {}
};

} // namespace CTRPluginFramework::lua