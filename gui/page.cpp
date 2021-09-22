#include "page.hpp"

#include "address_bar.hpp"
#include "connection.hpp"
#include "controller.hpp"
#include "monitor.hpp"
#include "resize_request.hpp"
#include "response.hpp"
#include "schedule.hpp"
#include "view.hpp"

#include <QVBoxLayout>
#include <QWidget>

namespace vision::gui {

namespace {

class Page final
  : public QWidget
  , public AddressBarObserver
  , public ConnectionObserver
  , public ResponseObserver
  , public ViewObserver
{
public:
  Page(QWidget* parent, Controller& controller)
    : QWidget(parent)
    , m_address_bar(CreateAddressBar(this, controller.GetURLsModel(), *this))
    , m_controller(controller)
  {
    m_layout.addWidget(m_address_bar);

    m_layout.addWidget(m_view, 1);

    m_layout.addWidget(m_monitor, 1);

    m_monitor->hide();
  }

private:
  void OnViewResize(size_t w,
                    size_t h,
                    size_t padded_w,
                    size_t padded_h) override
  {
    if (!m_connection)
      return;

    ResizeRequest resize_req{ w, h, padded_w, padded_h };

    IssueResize(resize_req);
  }

  void OnNewViewFrame() override
  {
    if (!m_connection)
      return;

    IssueRenderRequest();
  }

  void OnViewKeyEvent(const QString& key_text, bool state) override
  {
    if (!m_connection)
      return;

    const std::string key_text_utf8(key_text.toStdString());

    m_connection->SendKey(key_text_utf8, state);
  }

  void OnViewMouseMoveEvent(int x, int y) override
  {
    if (!m_connection)
      return;

    m_connection->SendMouseMove(x, y);
  }

  void OnViewMouseButtonEvent(const QString& button_name,
                              int x,
                              int y,
                              bool state) override
  {
    if (!m_connection)
      return;

    const std::string button_name_utf8(button_name.toStdString());

    m_connection->SendMouseButton(button_name_utf8, x, y, state);
  }

  void OnConnectionRequest(const QString& url) override
  {
    m_connection = m_controller.CreateConnection(url, *this);

    if (!m_connection->Connect())
      m_monitor->LogError("Failed to connect to \"" + url + "\".");
  }

  void OnMonitorVisibilityToggle(bool visible) override
  {
    m_monitor->setVisible(visible);
  }

  void OnConnectionStart() override
  {
    m_response_parser = ResponseParser::Create(*this);

    IssueResize(m_view->MakeResizeRequest());

    IssueRenderRequest();
  }

  void IssueResize(const ResizeRequest& req) { m_connection->Resize(req); }

  void IssueRenderRequest()
  {
    if (!m_connection) {
      m_monitor->LogError(
        "Render request was issued but there is no connection to send it to.");
      return;
    }

    const RenderRequest req = m_view->GetCurrentRenderRequest();

    if (!req.IsValid()) {
      m_monitor->LogError("Render request was issued but none was available.");
      return;
    }

    m_connection->Render(req);
  }

  void OnConnectionRecv(const unsigned char* buffer, size_t length) override
  {
    m_monitor->LogConnectionRead(length);

    m_response_parser->Write((const char*)buffer, length);
  }

  void OnInvalidResponse(const std::string_view& reason) override
  {
    QString message = QString("Invalid response: %1")
                        .arg(QString::fromUtf8(&reason[0], reason.size()));

    m_monitor->LogError(message);

    DisconnectDueToError();
  }

  void OnBufferOverflow() override
  {
    m_monitor->LogError("Buffer overflow occurred.");

    DisconnectDueToError();
  }

  void OnRGBBuffer(const unsigned char* buffer,
                   size_t w,
                   size_t h,
                   size_t request_id) override
  {
    if (!m_view->ReplyRenderRequest(buffer, w * h * 3, request_id)) {
      // This can happen when the window has been resized since the render
      // request was issued. It should not be considered an error.
#if 0
      m_monitor->LogError("Failed to reply to render request.");
      DisconnectDueToError();
#endif
    }

    if (m_view->HasRenderRequest())
      IssueRenderRequest();
  }

  void DisconnectDueToError()
  {
    m_monitor->LogInfo("Disconnecting due to error.");

    m_connection.reset();
  }

private:
  QWidget* m_address_bar;

  View* m_view{ CreateView(*this, this) };

  Monitor* m_monitor{ CreateMonitor(this) };

  QVBoxLayout m_layout{ this };

  Controller& m_controller;

  std::unique_ptr<Connection> m_connection;

  std::unique_ptr<ResponseParser> m_response_parser;
};

} // namespace

QWidget*
CreatePage(QWidget* parent, Controller& controller)
{
  return new Page(parent, controller);
}

} // namespace vision::gui