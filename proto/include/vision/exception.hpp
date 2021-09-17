#pragma once

#include <iosfwd>
#include <stdexcept>
#include <string>
#include <string_view>

namespace vision {

class Exception : public std::runtime_error
{
public:
  Exception(const char* what)
    : std::runtime_error(what)
  {}

  Exception(const std::string& what)
    : std::runtime_error(what)
  {}

  virtual ~Exception() = default;
};

class SyntaxError final : public Exception
{
public:
  SyntaxError(size_t line, size_t column, const std::string_view& msg);

  auto GetLine() const noexcept -> size_t { return m_line; }

  auto GetColumn() const noexcept -> size_t { return m_column; }

private:
  size_t m_line;

  size_t m_column;

  std::string m_msg;
};

std::ostream&
operator<<(std::ostream& output, const Exception&);

} // namespace vision
