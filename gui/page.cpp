#include "page.hpp"

#include "address_bar.hpp"

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace vision::gui {

namespace {

class InitialContentView final : public QLabel
{
public:
  InitialContentView(QWidget* parent)
    : QLabel(parent)
  {
    setText(tr("Connect to a renderer with the navigation bar above."));

    setAlignment(Qt::AlignCenter);
  }
};

class ContentArea : public QStackedWidget
{
public:
  ContentArea(QWidget* parent)
    : QStackedWidget(parent)
  {
    addWidget(&m_initial_content_view);
  }

private:
  InitialContentView m_initial_content_view{ this };
};

class Page final : public QWidget
{
public:
  Page(QWidget* parent)
    : QWidget(parent)
  {
    m_layout.addWidget(&m_address_bar);

    m_layout.addWidget(&m_content_area, 1);

    connect(&m_address_bar,
            &AddressBar::ConnectionRequest,
            this,
            &Page::OnConnectionRequest);
  }

protected slots:
  void OnConnectionRequest(const Address& address)
  {
#if 0
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
#endif
    (void)address;
  }

private:
  void OnMonitorVisibilityToggle(bool visible)
  {
    // m_monitor->setVisible(visible);

    (void)visible;
  }

  void DisconnectDueToError()
  {
#if 0
    m_monitor->LogInfo("Disconnecting due to error.");

    ResetConnection();
#endif
  }

private:
  AddressBar m_address_bar{ this };

  ContentArea m_content_area{ this };

  QVBoxLayout m_layout{ this };
};

} // namespace

QWidget*
CreatePage(QWidget* parent)
{
  return new Page(parent);
}

} // namespace vision::gui
