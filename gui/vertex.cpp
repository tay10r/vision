#include "vertex.hpp"

#include <ostream>

namespace vision::gui {

std::ostream&
operator<<(std::ostream& output, const Vertex& vertex)
{
  return output << '(' << vertex.x << ", " << vertex.y << ')';
}

} // namespace vision::gui
