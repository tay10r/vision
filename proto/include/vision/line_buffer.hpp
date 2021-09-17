#pragma once

#include <string>
#include <vector>

#include <stddef.h>

namespace vision {

class LineBuffer final
{
public:
  /// @return The number of bytes added to the line buffer. If the terminating
  ///         line is found before the end of the buffer, then the total number
  ///         of characters read stops at that point.
  size_t Write(const char* buffer, size_t buffer_size);

  size_t Write(const std::string& buffer)
  {
    return Write(buffer.data(), buffer.size());
  }

  auto GetLineNumber() const noexcept -> size_t { return m_line_number; }

  auto PopLine() -> std::string;

  auto GetAvailableLineCount() const -> size_t { return m_lines.size(); }

  auto IsTerminated() const -> bool
  {
    return !m_lines.empty() && (m_lines.back() == "\n");
  }

  size_t GetMaxSize() const noexcept
  {
    return m_max_line_count * m_max_line_size;
  }

private:
  bool AppendToCurrentLine(char c);

  bool AppendLine(std::string&& line);

private:
  std::string m_current_line;

  std::vector<std::string> m_lines;

  size_t m_line_number = 1;

  size_t m_max_line_count = 1024;

  size_t m_max_line_size = 8192;
};

} // namespace vision
