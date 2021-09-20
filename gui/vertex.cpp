#include "vertex.hpp"

#include <ostream>

namespace vision::gui {

std::ostream&
operator<<(std::ostream& output, const Vertex& vertex)
{
  const float x = vertex.x_pixel_offset + vertex.x_vertex_offset;
  const float y = vertex.y_pixel_offset + vertex.y_vertex_offset;
  return output << '(' << x << ", " << y << ')';
}

} // namespace vision::gui
