#include "fake_controller.hpp"

#include "connection.hpp"
#include "controller.hpp"

#include <iostream>
#include <random>
#include <vector>

#include <QObject>
#include <QString>
#include <QTimer>

namespace vision::gui {

namespace {

class DebugConnection final : public Connection
{
public:
  DebugConnection(ConnectionObserver& o)
    : m_observer(o)
  {
    std::cout << "Debug connection instanced." << std::endl;
  }

  bool Connect() override
  {
    m_observer.OnConnectionStart();

    return true;
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

  bool Connect() override { return false; }

  void Render(size_t, size_t, size_t, size_t, size_t, size_t) {}

  void Resize(size_t, size_t) override {}
};

class BufferOverflowConnection final
  : public Connection
  , public QObject
{
public:
  BufferOverflowConnection(ConnectionObserver& observer)
    : m_observer(observer)
  {
    m_timer.setInterval(10);

    connect(&m_timer,
            &QTimer::timeout,
            this,
            &BufferOverflowConnection::SendRandomData);
  }

  bool Connect() override
  {
    m_observer.OnConnectionStart();

    m_timer.start();

    return true;
  }

  void Render(size_t, size_t, size_t, size_t, size_t, size_t) {}

  void Resize(size_t, size_t) override {}

protected slots:
  void SendRandomData()
  {
    std::uniform_int_distribution<size_t> size_dist(4096, 8192);

    const size_t size = size_dist(m_rng);

    std::vector<unsigned char> buffer(size);

    std::uniform_int_distribution<unsigned char> byte_dist(0, 255);

    for (size_t i = 0; i < size; i++) {

      unsigned char c = byte_dist(m_rng);
      if (c == '\n') {
        // Avoid sending this because it causes a response line to be parsed.
        continue;
      }

      buffer[i] = c;
    }

    m_observer.OnConnectionRecv(&buffer[0], buffer.size());
  }

private:
  ConnectionObserver& m_observer;

  QTimer m_timer;

  std::mt19937 m_rng{ 1234 };
};

class FakeController final : public Controller
{
public:
  auto CreateConnection(const QString& url, ConnectionObserver& observer)
    -> std::unique_ptr<Connection> override
  {
    if (url == "debug")
      return std::unique_ptr<Connection>(new DebugConnection(observer));
    else if (url == "buffer overflow")
      return std::unique_ptr<Connection>(
        new BufferOverflowConnection(observer));
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
