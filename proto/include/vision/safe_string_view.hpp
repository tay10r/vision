#pragma once

#include <stddef.h>

namespace vision {

class SafeStringView final
{
public:
  constexpr SafeStringView() = default;

  constexpr SafeStringView(const char* data, size_t len)
    : m_data(data)
    , m_length(len)
  {}

  constexpr char operator[](size_t index) const noexcept
  {
    return (index < m_length) ? m_data[index] : 0;
  }

private:
  const char* m_data = nullptr;

  size_t m_length = 0;
};

} // namespace vision
