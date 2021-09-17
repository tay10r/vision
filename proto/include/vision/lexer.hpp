#pragma once

#include <vision/type_id.hpp>

#include <map>
#include <string>

namespace vision {

class SafeInput final
{
public:
  SafeInput(size_t max_buffer_size = 1048576 /* 1 MiB */);

  void Append(const char* text, size_t length);

  void PopChars(size_t count);

  auto Size() const noexcept -> size_t { return m_buffer.size(); }

  auto operator[](size_t i) const noexcept -> char
  {
    return (i < m_buffer.size()) ? m_buffer[i] : 0;
  }

private:
  std::string m_buffer;

  size_t m_max_buffer_size;
};

class Lexer final
{
public:
  Lexer();

  void Append(const char* text, size_t length) { m_input.Append(text, length); }

  void Append(const std::string& text)
  {
    m_input.Append(&text[0], text.size());
  }

  auto ScanIdentifier() -> std::string;

  auto ScanIdentifier(const std::string& expected) -> std::string;

  auto ScanType() -> const TypeID*;

  auto ScanSpace() -> std::string;

  auto ScanNewline() -> std::string;

  auto ScanSymbol(char c) -> std::string;

  auto ScanInteger() -> std::string;

  auto Remaining() const noexcept -> size_t { return m_input.Size(); }

private:
  auto PeekIdentifier() -> std::string;

  auto Produce(size_t length) -> std::string;

private:
  SafeInput m_input;

  std::map<std::string, TypeID> m_type_map;
};

} // namespace vision
