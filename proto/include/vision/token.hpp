#pragma once

#include <iosfwd>
#include <string_view>

#include <stddef.h>

namespace vision {

enum class TokenKind : size_t
{
  ID,
  Int,
  Flt,
  Space,
  Newline,
  Symbol
};

struct Token final
{
  TokenKind kind;

  std::string_view data;

  size_t line;

  size_t column;

  constexpr auto Size() const noexcept -> size_t { return data.size(); }

  constexpr bool operator==(const std::string_view& s) const noexcept
  {
    return data == s;
  }

  constexpr bool operator!=(const std::string_view& s) const noexcept
  {
    return data != s;
  }

  constexpr bool operator==(TokenKind k) const noexcept { return kind == k; }

  constexpr bool operator!=(TokenKind k) const noexcept { return kind != k; }

  constexpr bool operator==(char c) const noexcept
  {
    return (data.size() == 1) && (data[0] == c);
  }

  constexpr bool operator!=(char c) const noexcept
  {
    return (data.size() != 1) || (data[0] != c);
  }
};

std::ostream&
operator<<(std::ostream& output, const Token& token);

} // namespace vision
