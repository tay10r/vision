#include "debug_input.hpp"

#include <iostream>

namespace vision::gui {

namespace {

class InputDebugConnection final : public Connection
{
public:
  InputDebugConnection(ConnectionObserver& observer)
    : m_observer(observer)
  {}

  bool Connect() override
  {
    m_observer.OnConnectionStart();

    return true;
  }

  void Render(const RenderRequest&) override {}

  void Resize(const ResizeRequest&) override {}

  void SendKey(const std::string_view&, bool) override {}

  void SendMouseButton(const std::string_view&, int, int, bool) override {}

  void SendMouseMove(int x, int y) override
  {
    std::cout << x << ' ' << y << std::endl;
  }

private:
  ConnectionObserver& m_observer;
};

} // namespace

auto
CreateInputDebugConnection(ConnectionObserver& observer)
  -> std::unique_ptr<Connection>
{
  return std::unique_ptr<Connection>(new InputDebugConnection(observer));
}

} // namespace vision::gui
