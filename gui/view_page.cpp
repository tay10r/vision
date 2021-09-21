#include "view_page.hpp"

#include "address_bar.hpp"
#include "connection.hpp"
#include "controller.hpp"
#include "monitor.hpp"
#include "response.hpp"
#include "schedule.hpp"
#include "view.hpp"

#include <QVBoxLayout>
#include <QWidget>

namespace vision::gui {

namespace {

class ViewPage final
  : public QWidget
  , public AddressBarObserver
  , public ConnectionObserver
  , public ResponseObserver
{
public:
  ViewPage(QWidget* parent, Controller& controller)
    : QWidget(parent)
    , m_controller(controller)
  {
    m_layout.addWidget(m_address_bar);

    m_layout.addWidget(m_view, 1);

    m_layout.addWidget(m_monitor, 1);
  }

private:
  void OnConnectionRequest(const QString& url) override
  {
    m_connection = m_controller.CreateConnection(url, *this);

    if (!m_connection->Connect())
      m_monitor->LogError("Failed to connect to \"" + url + "\".");
  }

  void OnLogVisibilityChange(bool visible) override
  {
    m_monitor->setVisible(visible);
  }

  void OnConnectionStart() override
  {
    m_response_parser = ResponseParser::Create(*this);

    const QSize view_size = m_view->size();

    m_connection->Resize(view_size.width(), view_size.height());

    IssueRenderRequest();
  }

  void IssueRenderRequest()
  {
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

  void OnRGBBuffer(const unsigned char* buffer, size_t w, size_t h) override
  {
    if (!m_view->ReplyRenderRequest(buffer, w * h * 3)) {
      m_monitor->LogError("Failed to reply to render request.");
      DisconnectDueToError();
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
  QWidget* m_address_bar{ CreateAddressBar(this, *this) };

  View* m_view{ CreateView(this) };

  Monitor* m_monitor{ CreateMonitor(this) };

  QVBoxLayout m_layout{ this };

  Controller& m_controller;

  std::unique_ptr<Connection> m_connection;

  std::unique_ptr<ResponseParser> m_response_parser;
};

} // namespace

QWidget*
CreateViewPage(QWidget* parent, Controller& controller)
{
  return new ViewPage(parent, controller);
}

} // namespace vision::gui
