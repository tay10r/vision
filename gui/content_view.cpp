#include "page.hpp"

#include "address_bar.hpp"
#include "connection.hpp"
#include "connection_view_proxy.hpp"
#include "debug_connection.hpp"
#include "monitor.hpp"
#include "resize_request.hpp"
#include "response.hpp"
#include "schedule.hpp"
#include "view.hpp"

#include <QVBoxLayout>
#include <QWidget>

#include <QDebug>

namespace vision::gui {

namespace {

class ConnectionContext final
{
public:
  ConnectionContext(std::unique_ptr<Connection>&& connection, View* view)
    : m_connection(std::move(connection))
    , m_connection_view_proxy(CreateConnectionViewProxy(*m_connection))
    , m_view(view)
  {
    view->SetObserver(m_connection_view_proxy.get());
  }

  ~ConnectionContext() { m_view->SetObserver(nullptr); }

  Connection* GetConnection() { return m_connection.get(); }

private:
  std::unique_ptr<Connection> m_connection;

  std::unique_ptr<ViewObserver> m_connection_view_proxy;

  View* m_view;
};

class Page final
  : public QWidget
  , public ConnectionObserver
  , public ResponseObserver
{
public:
  Page(QWidget* parent)
    : QWidget(parent)
  {
    m_layout.addWidget(&m_address_bar);

    m_layout.addWidget(m_view, 1);

    m_layout.addWidget(m_monitor, 1);

    m_monitor->hide();

    connect(&m_address_bar,
            &AddressBar::ConnectionRequest,
            this,
            &Page::OnConnectionRequest);
  }

protected slots:
  void OnConnectionRequest(const Address& address)
  {
    ResetConnection();

    switch (address.kind) {
      case AddressKind::Debug:
        CreateConnection(CreateDebugConnection(*this, address.data));
        break;
      case AddressKind::Tcp:
        break;
      case AddressKind::File:
        break;
      case AddressKind::Unknown:
        break;
    }

    Connection* connection = GetConnection();
    if (connection) {
      connection->Connect();
    }
  }

private:
  void OnMonitorVisibilityToggle(bool visible)
  {
    m_monitor->setVisible(visible);
  }

  void OnConnectionStart() override
  {
    qDebug() << "here";

    m_response_parser = ResponseParser::Create(*this);

    const ResizeRequest resize_req = m_view->MakeResizeRequest();

    Connection* connection = GetConnection();
    if (connection) {
      qDebug() << "here 2";
      connection->Resize(resize_req);
    }

    m_view->NewFrame();
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
  }

  void DisconnectDueToError()
  {
    m_monitor->LogInfo("Disconnecting due to error.");

    ResetConnection();
  }

  bool CreateConnection(std::unique_ptr<Connection>&& conn)
  {
    if (!ResetConnection(new ConnectionContext(std::move(conn), m_view))) {
      // TODO : error
      return false;
    }

    return true;
  }

  bool ResetConnection(ConnectionContext* context = nullptr)
  {
    m_connection_context.reset(context);

    return context != nullptr;
  }

  Connection* GetConnection()
  {
    if (m_connection_context)
      return m_connection_context->GetConnection();
    else
      return nullptr;
  }

private:
  AddressBar m_address_bar{ this };

  View* m_view{ CreateView(this) };

  Monitor* m_monitor{ CreateMonitor(this) };

  QVBoxLayout m_layout{ this };

  std::unique_ptr<ConnectionContext> m_connection_context;

  std::unique_ptr<ResponseParser> m_response_parser;
};

} // namespace

QWidget*
CreatePage(QWidget* parent)
{
  return new Page(parent);
}

} // namespace vision::gui
