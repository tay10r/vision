#include <vision/exception.hpp>

#include <sstream>

namespace vision {

namespace {

std::string
Format(size_t ln, size_t col, const std::string_view& msg)
{
  std::ostringstream tmp_stream;
  tmp_stream << '(';
  tmp_stream << "ln:" << ln;
  tmp_stream << ", ";
  tmp_stream << "col:" << col;
  tmp_stream << "): " << msg;
  return tmp_stream.str();
}

} // namespace

SyntaxError::SyntaxError(size_t ln, size_t col, const std::string_view& msg)
  : Exception(Format(ln, col, msg))
  , m_line(ln)
  , m_column(col)
  , m_msg(msg)
{}

std::ostream&
operator<<(std::ostream& output, const Exception& exception)
{
  return output << exception.what();
}

} // namespace vision
