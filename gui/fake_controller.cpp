#include "fake_controller.hpp"

#include "connection.hpp"
#include "controller.hpp"
#include "render_request.hpp"

#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#include <QObject>
#include <QString>
#include <QTimer>

namespace vision::gui {

namespace {

struct Circle final
{
  float u;
  float v;
  float r;

  bool Contains(float other_u, float other_v) const noexcept
  {
    const float a = (u - other_u);
    const float b = (v - other_v);

    return ((a * a) + (b * b)) < (r * r);
  }
};

class DebugConnection final
  : public Connection
  , public QObject
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

  void Render(const RenderRequest& req) override
  {
    m_render_requests.emplace_back(req);

    QTimer::singleShot(10, this, &DebugConnection::HandleFirstRenderRequest);
  }

  void Resize(size_t w, size_t h) override
  {
    m_width = std::max(w, size_t(1));
    m_height = std::max(h, size_t(1));
    std::cout << "Resizing to (" << w << ", " << h << ")" << std::endl;
  }

protected slots:
  void HandleFirstRenderRequest()
  {
    if (m_render_requests.empty())
      return;

    HandleRenderRequest(m_render_requests[0]);

    m_render_requests.erase(m_render_requests.begin());
  }

private:
  void HandleRenderRequest(const RenderRequest& req)
  {
    std::uniform_real_distribution<float> color_dist(0.0f, 1.0f);

    const float color[3]{ color_dist(m_rng),
                          color_dist(m_rng),
                          color_dist(m_rng) };

    std::vector<unsigned char> buffer(req.x_pixel_count * req.y_pixel_count *
                                      3);

    const float x_scale = 1.0f / m_width;
    const float y_scale = 1.0f / m_height;

    Circle circle{ 0.5, 0.5, 0.3 };

    for (size_t y = 0; y < req.y_pixel_count; y++) {

      for (size_t x = 0; x < req.x_pixel_count; x++) {

        const float u = (req.GetFrameX(x) + 0.5f) * x_scale;
        const float v = (req.GetFrameY(y) + 0.5f) * y_scale;

        const float k = circle.Contains(u, v) ? 1.0f : 0.0f;

        size_t buffer_offset = ((y * req.x_pixel_count) + x) * 3;

        buffer[buffer_offset + 0] = color[0] * k * 255;
        buffer[buffer_offset + 1] = color[1] * k * 255;
        buffer[buffer_offset + 2] = color[2] * k * 255;
      }
    }

    std::ostringstream header_stream;

    header_stream << "rgb buffer " << req.x_pixel_count << ' '
                  << req.y_pixel_count << '\n';

    std::string header = header_stream.str();

    m_observer.OnConnectionRecv((const unsigned char*)&header[0],
                                header.size());

    m_observer.OnConnectionRecv(&buffer[0], buffer.size());
  }

private:
  std::vector<RenderRequest> m_render_requests;

  ConnectionObserver& m_observer;

  std::mt19937 m_rng{ 1234 };

  size_t m_width = 1;

  size_t m_height = 1;
};

class BadConnection final : public Connection
{
public:
  BadConnection() { std::cout << "Bad connection instanced." << std::endl; }

  bool Connect() override { return false; }

  void Render(const RenderRequest&) override {}

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

  void Render(const RenderRequest&) override {}

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
