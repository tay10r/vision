#include "view_page.hpp"

#include "address_bar.hpp"
#include "connection.hpp"
#include "controller.hpp"
#include "monitor.hpp"
#include "view.hpp"

#include <QVBoxLayout>
#include <QWidget>

namespace vision::gui {

namespace {

class ViewPage final
  : public QWidget
  , public AddressBarObserver
  , public ConnectionObserver
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
    m_connection = m_controller.Connect(url, *this);
  }

  void OnLogVisibilityChange(bool visible) override
  {
    m_monitor->setVisible(visible);
  }

  void OnConnectionRecv(const unsigned char*, size_t) override
  {
    //
  }

private:
  QWidget* m_address_bar{ CreateAddressBar(this, *this) };

  QWidget* m_view{ CreateView(this) };

  Monitor* m_monitor{ CreateMonitor(this) };

  QVBoxLayout m_layout{ this };

  Controller& m_controller;

  std::unique_ptr<Connection> m_connection;
};

} // namespace

QWidget*
CreateViewPage(QWidget* parent, Controller& controller)
{
  return new ViewPage(parent, controller);
}

} // namespace vision::gui
