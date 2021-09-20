#pragma once

#include <iosfwd>

namespace vision::gui {

struct Vertex final
{
  float x_pixel_offset = 0;
  float y_pixel_offset = 0;
  float x_vertex_offset = 0;
  float y_vertex_offset = 0;

  constexpr Vertex() noexcept = default;

  template<typename Scalar>
  constexpr Vertex(Scalar a, Scalar b, Scalar c, Scalar d) noexcept
    : x_pixel_offset(a)
    , y_pixel_offset(b)
    , x_vertex_offset(c)
    , y_vertex_offset(d)
  {}
};

std::ostream&
operator<<(std::ostream&, const Vertex&);

} // namespace vision::gui
