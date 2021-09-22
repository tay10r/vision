#pragma once

namespace vision::gui {

class IDGenerator final
{
public:
  constexpr size_t GenerateID() noexcept { return m_next++; }

private:
  size_t m_next = 0;
};

} // namespace vision::gui
