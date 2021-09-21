#include "token.hpp"

namespace vision::gui {

std::ostream&
operator<<(std::ostream& output, const Token& token)
{
  return output << token.data;
}

} // namespace vision::gui
