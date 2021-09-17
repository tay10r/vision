#pragma once

#include <stddef.h>

namespace vision {

enum class TypeID
{
  Int,
  Float,
  Vec2,
  Vec3,
  Vec4,
  Vec2i,
  Vec3i,
  Vec4i
};

constexpr auto
ToString(TypeID type_id) -> const char*
{
  switch (type_id) {
    case TypeID::Int:
      return "int";
    case TypeID::Float:
      return "float";
    case TypeID::Vec2:
      return "vec2";
    case TypeID::Vec3:
      return "vec3";
    case TypeID::Vec4:
      return "vec4";
    case TypeID::Vec2i:
      return "vec2i";
    case TypeID::Vec3i:
      return "vec3i";
    case TypeID::Vec4i:
      return "vec4i";
  }

  return "";
}

constexpr auto
GetMemberCount(TypeID type_id) -> size_t
{
  switch (type_id) {
    case TypeID::Int:
    case TypeID::Float:
      return 1;
    case TypeID::Vec2:
    case TypeID::Vec2i:
      return 2;
    case TypeID::Vec3:
    case TypeID::Vec3i:
      return 3;
    case TypeID::Vec4:
    case TypeID::Vec4i:
      return 4;
  }

  return 0;
}

} // namespace vision
