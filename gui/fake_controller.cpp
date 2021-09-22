#include "fake_controller.hpp"

#include "connection.hpp"
#include "controller.hpp"
#include "debug_render.hpp"
#include "render_request.hpp"

#include <random>
#include <sstream>
#include <vector>

#include <QObject>
#include <QString>
#include <QStringListModel>
#include <QTimer>
#include <QUrl>

#include <QDebug>

namespace vision::gui {

namespace {

class BadConnection final : public Connection
{
public:
  BadConnection(ConnectionObserver& observer)
    : m_observer(observer)
  {}

  bool Connect() override { return false; }

  void Render(const RenderRequest&) override {}

  void Resize(const ResizeRequest&) override {}

  void SendKey(const std::string_view&, bool) override {}

  void SendMouseButton(const std::string_view&, int, int, bool) override {}

  void SendMouseMove(int, int) override {}

private:
  ConnectionObserver& m_observer;
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

  void Resize(const ResizeRequest&) override {}

  void SendKey(const std::string_view&, bool) override {}

  void SendMouseButton(const std::string_view&, int, int, bool) override {}

  void SendMouseMove(int, int) override {}

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

class NullConnection final : public Connection
{
public:
  NullConnection(ConnectionObserver& observer)
    : m_observer(observer)
  {}

  bool Connect() override { return false; }

  void Render(const RenderRequest&) override {}

  void Resize(const ResizeRequest&) override {}

  void SendKey(const std::string_view&, bool) override {}

  void SendMouseButton(const std::string_view&, int, int, bool) override {}

  void SendMouseMove(int, int) override {}

private:
  ConnectionObserver& m_observer;
};

class URLsModel final
{
public:
  URLsModel()
  {
    QStringList builtin_urls;
    builtin_urls << "debug://bad_connection";
    builtin_urls << "debug://buffer_overflow";
    builtin_urls << "debug://render";
    m_model.setStringList(builtin_urls);
  }

  QStringListModel* GetModel() { return &m_model; }

private:
  QStringListModel m_model;
};

class ConnectionFactory final
{
public:
  auto CreateConnection(const QString& urlString, ConnectionObserver& observer)
    -> std::unique_ptr<Connection>
  {
    const QUrl url(urlString);

    if (!url.isValid())
      return std::unique_ptr<Connection>(new BadConnection(observer));

    const QString scheme = url.scheme();

    ConnectionPtr connection;

    if (scheme == "file")
      connection = CreateProgramConnection(url, observer);
    else if (scheme == "debug")
      connection = CreateDebugConnection(url, observer);
    else
      connection = CreateTCPConnection(url, observer);

    if (!connection)
      connection.reset(new NullConnection(observer));

    return connection;
  }

private:
  using ConnectionPtr = std::unique_ptr<Connection>;

  auto CreateDebugConnection(const QUrl& url, ConnectionObserver& observer)
    -> ConnectionPtr
  {
    const QString debug_kind = url.toString();

    if (debug_kind == "debug://bad_connection")
      return ConnectionPtr(new BadConnection(observer));
    else if (debug_kind == "debug://buffer_overflow")
      return ConnectionPtr(new BufferOverflowConnection(observer));
    else if (debug_kind == "debug://render")
      return ConnectionPtr(CreateDebugRenderer(observer));
    else
      return nullptr;
  }

  auto CreateProgramConnection(const QUrl&, ConnectionObserver&)
    -> ConnectionPtr
  {
    // TODO

    return nullptr;
  }

  auto CreateTCPConnection(const QUrl&, ConnectionObserver&) -> ConnectionPtr
  {
    //

    return nullptr;
  }
};

class FakeController final
  : public Controller
  , public QObject
{
public:
  QAbstractItemModel* GetURLsModel() override { return urls_model.GetModel(); }

  auto CreateConnection(const QString& url, ConnectionObserver& observer)
    -> std::unique_ptr<Connection> override
  {
    return m_connection_factory.CreateConnection(url, observer);
  }

private:
  URLsModel urls_model;

  ConnectionFactory m_connection_factory;
};

} // namespace

std::unique_ptr<Controller>
Controller::Create()
{
  return std::unique_ptr<Controller>(new FakeController());
}

} // namespace vision::gui
