#include "fake_controller.hpp"

#include "connection.hpp"
#include "controller.hpp"

#include <iostream>
#include <vector>

#include <QString>

namespace vision::gui {

namespace {

class DebugConnection final : public Connection
{
public:
  DebugConnection(ConnectionObserver& o)
    : m_observer(o)
  {
    std::cout << "Debug connection instanced." << std::endl;

    m_observer.OnConnectionStart();
  }

  void Render(size_t w,
              size_t h,
              size_t x_offset,
              size_t y_offset,
              size_t x_stride,
              size_t y_stride) override
  {
    std::vector<unsigned char> buffer(w * h * 3);

    (void)w;
    (void)h;
    (void)x_offset;
    (void)y_offset;
    (void)x_stride;
    (void)y_stride;

    m_observer.OnConnectionRecv(&buffer[0], buffer.size());
  }

  void Resize(size_t w, size_t h) override
  {
    m_width = w;

    m_height = h;

    std::cout << "Resizing to (" << w << ", " << h << ")" << std::endl;
  }

private:
  ConnectionObserver& m_observer;

  size_t m_width = 0;

  size_t m_height = 0;
};

class BadConnection final : public Connection
{
public:
  BadConnection() { std::cout << "Bad connection instanced." << std::endl; }

  void Render(size_t, size_t, size_t, size_t, size_t, size_t) {}

  void Resize(size_t, size_t) override {}
};

class FakeController final : public Controller
{
public:
  auto Connect(const QString& url, ConnectionObserver& observer)
    -> std::unique_ptr<Connection> override
  {
    if (url == "debug")
      return std::unique_ptr<Connection>(new DebugConnection(observer));
    else
      return std::unique_ptr<Connection>(new BadConnection());
  }
};

} // namespace

std::unique_ptr<Controller>
Controller::Create()
{
  return std::unique_ptr<Controller>(new FakeController());
}

} // namespace vision::gui
