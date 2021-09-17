#pragma once

#include <vision/token.hpp>

#include <optional>

namespace vision {

class Lexer final
{
public:
  Lexer(const std::string_view& input);

  auto AtEnd() const noexcept -> bool;

  auto Remaining() const noexcept -> size_t;

  auto Scan() -> std::optional<Token>;

private:
  auto ScanIdentifier() -> std::optional<Token>;

  auto ScanIdentifier(const std::string_view& expected) -> std::optional<Token>;

  auto ScanSpace() -> std::optional<Token>;

  auto ScanNewline() -> std::optional<Token>;

  auto ScanSymbol(char c) -> std::optional<Token>;

  auto ScanInteger() -> std::optional<Token>;

  auto PeekIdentifier() -> std::optional<std::string_view>;

  auto Peek(size_t offset) const noexcept -> std::optional<char>;

  auto Produce(TokenKind kind, size_t length) -> std::optional<Token>;

  void Advance(size_t count);

private:
  std::string_view m_input;

  size_t m_input_offset = 0;

  size_t m_column = 1;

  size_t m_line = 1;
};

} // namespace vision
