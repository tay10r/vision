#include "main.hpp"

#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QSizePolicy>
#include <QTabBar>
#include <QToolButton>

#include <algorithm>
#include <memory>

#include <stdlib.h>

#define DEFAULT_PORT 3141

int
main(int argc, char** argv)
{
  QApplication app(argc, argv);

  navi::MainWindow main_window;

  main_window.show();

  return app.exec();
}

namespace navi {

namespace {

void
SetButtonSizePolicy(QPushButton& button)
{
  button.setSizePolicy(QSizePolicy::Minimum,
                       button.sizePolicy().verticalPolicy());
}

} // namespace

//=============//
// Address Bar //
//=============//

AddressBar::AddressBar(QWidget* parent)
  : QWidget(parent)
{
  m_layout.addWidget(&m_refresh_button);

  m_layout.addWidget(&m_line_edit);

  m_layout.addWidget(&m_go_button);

  m_layout.addWidget(&m_menu_button);

  m_line_edit.setPlaceholderText(tr("Enter an address to connect to."));

  SetButtonSizePolicy(m_refresh_button);

  SetButtonSizePolicy(m_menu_button);

  setSizePolicy(sizePolicy().horizontalPolicy(), QSizePolicy::Minimum);

  connect(
    &m_go_button, &QPushButton::clicked, this, &AddressBar::OnGoButtonClicked);
}

void
AddressBar::OnGoButtonClicked()
{
  auto url = QUrl::fromUserInput(m_line_edit.text());

  if (url.isValid()) {
    emit ConnectionRequest(url);
  } else {
    // TODO
  }
}

//===============//
// Render Client //
//===============//

RenderClient::RenderClient()
  : m_tcp_socket(this)
{}

QTcpSocket*
RenderClient::GetTcpSocket()
{
  return &m_tcp_socket;
}

const QTcpSocket*
RenderClient::GetTcpSocket() const
{
  return &m_tcp_socket;
}

void
RenderClient::Connect(const QUrl& url)
{
  m_tcp_socket.connectToHost(url.host(), url.port(DEFAULT_PORT));
}

//======//
// View //
//======//

View::View(QWidget* parent)
  : QOpenGLWidget(parent)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

//=====//
// Log //
//=====//

Log::Log(QWidget* parent)
  : QTextEdit(parent)
{
  setReadOnly(true);
}

Log&
Log::operator<<(const char* str)
{
  insertPlainText(QString(str));
  return *this;
}

Log&
Log::operator<<(const QString& str)
{
  insertPlainText(str);
  return *this;
}

Log&
Log::operator<<(int number)
{
  insertPlainText(QString::number(number));
  return *this;
}

//===========//
// View Page //
//===========//

ViewPage::ViewPage(QWidget* parent)
  : QWidget(parent)
{
  m_layout.addWidget(&m_address_bar);

  m_layout.addWidget(&m_view);

  m_layout.addWidget(&m_log);

  connect(&m_address_bar,
          &AddressBar::ConnectionRequest,
          this,
          &ViewPage::OnConnectionRequest);
}

void
ViewPage::OnConnectionRequest(const QUrl& url)
{
  m_log << "Connecting to \"" << url.host() << "\".\n";

  m_client.reset(new RenderClient());

  QTcpSocket* socket = m_client->GetTcpSocket();

  connect(socket, &QTcpSocket::hostFound, this, &ViewPage::OnHostFound);

  connect(socket, &QTcpSocket::connected, this, &ViewPage::OnConnected);

  connect(socket, &QTcpSocket::disconnected, this, &ViewPage::OnDisconnected);

  connect(socket, &QTcpSocket::errorOccurred, this, &ViewPage::OnErrorOccurred);

  connect(socket, &QTcpSocket::readyRead, this, &ViewPage::OnReadReady);

  m_client->Connect(url);
}

void
ViewPage::OnHostFound()
{
  m_log << "Found host.\n";
}

void
ViewPage::OnConnected()
{
  m_log << "Connected to host.\n";
}

void
ViewPage::OnDisconnected()
{
  m_log << "Disconnected from host.\n";
}

void
ViewPage::OnErrorOccurred(QAbstractSocket::SocketError error)
{
  switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
      m_log << "Connection refused.\n";
      break;
    case QAbstractSocket::RemoteHostClosedError:
      m_log << "Remote host closed.\n";
      break;
    case QAbstractSocket::HostNotFoundError:
      m_log << "Host not found.\n";
      break;
    case QAbstractSocket::NetworkError:
      m_log << "Network error occurred.\n";
      break;
    default:
      m_log << "Unknown socket error occurred.\n";
      break;
  }
}

void
ViewPage::OnReadReady()
{
  QTcpSocket* socket = m_client->GetTcpSocket();

  qint64 bytes_available = socket->bytesAvailable();

  if (bytes_available < 0) {
    return;
  }

  qint64 bytes_to_read = std::min(bytes_available, m_max_read_size);

  std::vector<char> buffer(bytes_to_read);

  qint64 read_size = socket->read(&buffer[0], bytes_to_read);

  if (read_size < 0)
    return;

  buffer.resize(read_size);
}

bool
ViewPage::HasClient() const noexcept
{
  return !!m_client;
}

//============//
// MainWindow //
//============//

MainWindow::MainWindow()
{
  setWindowTitle("Navi");

  setCentralWidget(&m_tabs_widget);

  resize(1280, 720);

  m_tabs_widget.setTabsClosable(true);

  m_tabs_widget.tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);

  connect(&m_tabs_widget,
          &QTabWidget::tabBarClicked,
          this,
          &MainWindow::OnTabBarClicked);

  connect(&m_tabs_widget,
          &QTabWidget::tabCloseRequested,
          this,
          &MainWindow::OnTabCloseRequest);

  AddPage(new ViewPage(this));
}

void
MainWindow::AddPage(ViewPage* page)
{
  m_pages.emplace_back(page);

  if (m_tabs_widget.count() > 0)
    m_tabs_widget.removeTab(m_tabs_widget.count() - 1);

  m_tabs_widget.addTab(page, "New Tab");

  AddNewTabButton();
}

void
MainWindow::AddNewTabButton()
{
  QLabel* new_tab_button = new QLabel(&m_tabs_widget);

  new_tab_button->setText("+");

  QLabel* add_tabs_label =
    new QLabel(tr("Add tabs by pressing \"+\""), &m_tabs_widget);

  add_tabs_label->setAlignment(Qt::AlignCenter);

  m_tabs_widget.addTab(add_tabs_label, QString());

  const int add_tab_index = m_tabs_widget.count() - 1;

  m_tabs_widget.tabBar()->setTabButton(
    add_tab_index, QTabBar::RightSide, new_tab_button);
}

void
MainWindow::OnTabBarClicked(int tabIndex)
{
  if ((tabIndex >= 0) && (tabIndex == (m_tabs_widget.count() - 1)))
    AddPage(new ViewPage(this));
}

void
MainWindow::OnTabCloseRequest(int tabIndex)
{
  m_pages.erase(m_pages.begin() + tabIndex);

  m_tabs_widget.removeTab(tabIndex);
}

} // namespace navi
