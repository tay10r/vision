#include "connection_view_proxy.hpp"

#include "connection.hpp"
#include "render_request.hpp"
#include "resize_request.hpp"
#include "schedule.hpp"
#include "view.hpp"

#include <QDebug>

namespace vision::gui {

namespace {

class ConnectionViewProxy final : public ViewObserver
{
public:
  ConnectionViewProxy(Connection& connection)
    : m_connection(connection)
  {}

  void OnNewFrame(const Schedule& schedule) override
  {
    const size_t req_count = schedule.GetRenderRequestCount();

    for (size_t i = 0; i < req_count; i++) {

      const RenderRequest req = schedule.GetRenderRequest(i);

      m_connection.Render(req);
    }
  }

  void OnMouseMoveEvent(int, int) override {}

  void OnMouseButtonEvent(const QString&, int, int, bool) override {}

  void OnKeyEvent(const QString&, bool) override {}

  void OnResize(size_t w, size_t h, size_t padded_w, size_t padded_h) override
  {
    qDebug() << "resize";

    const ResizeRequest resize_req{ w, h, padded_w, padded_h };

    m_connection.Resize(resize_req);
  }

private:
  Connection& m_connection;
};

} // namespace

auto
CreateConnectionViewProxy(Connection& connection)
  -> std::unique_ptr<ViewObserver>
{
  return std::unique_ptr<ViewObserver>(new ConnectionViewProxy(connection));
}

} // namespace vision::gui
