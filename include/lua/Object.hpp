#pragma once

#include "TypeInfo.hpp"

#include "utf.hpp"

namespace CTRPluginFramework::lua {

struct Object {

  TypeInfo type;

  union {
    i32 v_i32 = 0;
    u32 v_u32;
    float v_float;
    bool v_bool;
    std::u16string *v_str;
  };

  string to_str()const{
    switch(type.kind){
      case TypeKind::I32:return std::to_string(v_i32);
      case TypeKind::U32:return std::to_string(v_u32);
      case TypeKind::Float:return std::to_string(v_float);
      case TypeKind::Bool:return v_bool?"true":"false";
      case TypeKind::Str:return utf::utf16_to_utf8(*v_str);
    }
    return "??";
  }

  Object() {}

  Object(TypeInfo type) : type(type) {}

  ~Object() {}
};

} // namespace CTRPluginFramework::lua