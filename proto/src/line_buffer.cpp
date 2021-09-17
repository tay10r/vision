#include <vision/line_buffer.hpp>

#include <vision/safe_string_view.hpp>

namespace vision {

size_t
LineBuffer::Write(const char* buffer, size_t buffer_size)
{
  SafeStringView buffer_view(buffer, buffer_size);

  size_t read_count = 0;

  while ((read_count < buffer_size) && !IsTerminated()) {

    if (buffer_view[read_count] == '\n') {
      AppendToCurrentLine('\n');
      read_count += 1;
      continue;
    }

    if ((buffer_view[read_count] == '\r') &&
        (buffer_view[read_count + 1] == '\n')) {
      AppendToCurrentLine('\n');
      read_count += 2;
      continue;
    }

    if (buffer_view[read_count] == '\r') {
      AppendToCurrentLine('\n');
      read_count += 1;
      continue;
    }

    AppendToCurrentLine(buffer_view[read_count]);

    read_count++;
  }

  return read_count;
}

std::string
LineBuffer::PopLine()
{
  if (m_lines.empty())
    return "";

  std::string line = std::move(m_lines[0]);

  m_lines.erase(m_lines.begin());

  m_line_number++;

  return line;
}

bool
LineBuffer::AppendLine(std::string&& line)
{
  if (m_line_number < m_max_line_count) {
    m_lines.emplace_back(std::move(line));
    return true;
  } else {
    return false;
  }
}

bool
LineBuffer::AppendToCurrentLine(char c)
{
  if ((m_current_line.size() < m_max_line_size) || (c == '\n')) {
    m_current_line.push_back(c);
    if (c == '\n')
      return AppendLine(std::move(m_current_line));
    else
      return true;
  } else {
    return false;
  }
}

} // namespace vision
