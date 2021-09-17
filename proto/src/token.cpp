#include <vision/token.hpp>

namespace vision {

std::ostream&
operator<<(std::ostream& output, const Token& token)
{
  return output << token.data;
}

} // namespace vision
