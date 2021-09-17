#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QOpenGLWidget>
#include <QPushButton>
#include <QTcpSocket>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

namespace navi {

class AddressBar : public QWidget
{
  Q_OBJECT
public:
  AddressBar(QWidget* parent);

signals:
  void ConnectionRequest(const QUrl& url);

protected slots:

  void OnGoButtonClicked();

private:
  QHBoxLayout m_layout{ this };

  QLineEdit m_line_edit{ this };

  QPushButton m_refresh_button{ "\u27f3", this };

  QPushButton m_go_button{ "\u2192", this };

  QPushButton m_menu_button{ "\u2630", this };
};

class RenderClient : public QObject
{
public:
  RenderClient();

  QTcpSocket* GetTcpSocket();

  const QTcpSocket* GetTcpSocket() const;

  void Connect(const QUrl& url);

private:
  QTcpSocket m_tcp_socket;
};

class View : public QOpenGLWidget
{
public:
  View(QWidget* parent);
};

class Log : public QTextEdit
{
public:
  Log(QWidget* parent);

  Log& operator<<(const char* text);

  Log& operator<<(const QString& text);

  Log& operator<<(int n);
};

class ViewPage : public QWidget
{
  Q_OBJECT
public:
  ViewPage(QWidget* parent);

  bool HasClient() const noexcept;

protected slots:
  void OnConnectionRequest(const QUrl& url);

  void OnHostFound();

  void OnConnected();

  void OnDisconnected();

  void OnErrorOccurred(QAbstractSocket::SocketError);

  void OnReadReady();

private:
  QVBoxLayout m_layout{ this };

  AddressBar m_address_bar{ this };

  View m_view{ this };

  Log m_log{ this };

  std::unique_ptr<RenderClient> m_client;

  qint64 m_max_read_size = 1024 * 1024;
};

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow();

private:
  void AddPage(ViewPage* page);

  void AddNewTabButton();

protected slots:
  void OnTabBarClicked(int tabIndex);

  void OnTabCloseRequest(int tabIndex);

private:
  QTabWidget m_tabs_widget{ this };

  std::vector<ViewPage*> m_pages;
};

} // namespace navi
