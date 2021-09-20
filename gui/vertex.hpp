#pragma once

#include <iosfwd>

namespace vision::gui {

struct Vertex final
{
  float x = 0;
  float y = 0;

  constexpr Vertex() noexcept = default;

  template<typename Scalar>
  constexpr Vertex(Scalar a, Scalar b) noexcept
    : x(a)
    , y(b)
  {}

  constexpr bool operator==(const Vertex& other) const noexcept
  {
    return (x == other.x) && (y == other.y);
  }

  constexpr bool operator!=(const Vertex& other) const noexcept
  {
    return (x != other.x) || (y != other.y);
  }
};

std::ostream&
operator<<(std::ostream&, const Vertex&);

} // namespace vision::gui
