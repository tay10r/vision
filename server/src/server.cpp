#include <vision/server.hpp>

#include <vision/server_observer.hpp>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iostream>

namespace vision {

namespace {

class DefaultServerObserver final : public ServerObserver
{
public:
  static ServerObserver& GetInstance()
  {
    static DefaultServerObserver observer;
    return observer;
  }

  DefaultServerObserver(std::ostream& os = std::cout,
                        std::ostream& es = std::cerr)
    : m_out_stream(os)
    , m_err_stream(es)
  {}

  void OnAddressParseFailure(const char* addr) override
  {
    m_err_stream << "Not a valid address: \"" << addr << "\"." << std::endl;
  }

  void OnConnection() override
  {
    m_out_stream << "Connection made." << std::endl;
  }

  void OnStart() override { m_out_stream << "Server started." << std::endl; }

  void OnShutdown() override
  {
    m_err_stream << "Server shutting down." << std::endl;
  }

private:
  std::ostream& m_out_stream;
  std::ostream& m_err_stream;
};

class TcpConnection : public boost::enable_shared_from_this<TcpConnection>
{
public:
  using Observer = ServerObserver;

  using Pointer = boost::shared_ptr<TcpConnection>;

  static auto Create(Observer& o, boost::asio::io_context& io_context)
    -> Pointer
  {
    return Pointer(new TcpConnection(o, io_context));
  }

  boost::asio::ip::tcp::socket& GetSocket() { return m_socket; }

  void Start()
  {
    m_observer.OnConnection();

    m_message = "Hello, world!";

    boost::asio::async_write(
      m_socket,
      boost::asio::buffer(m_message),
      boost::bind(&TcpConnection::HandleWrite,
                  shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
  }

private:
  TcpConnection(Observer& o, boost::asio::io_context& io_context)
    : m_observer(o)
    , m_socket(io_context)
  {}

  void HandleWrite(const boost::system::error_code& /*error*/,
                   size_t /*bytes_transferred*/)
  {}

  Observer& m_observer;

  boost::asio::ip::tcp::socket m_socket;

  std::string m_message;
};

} // namespace

class ServerImpl final
{
  friend Server;

  using Observer = ServerObserver;

  Observer& m_observer;

  boost::asio::io_context m_io_context;

  boost::asio::ip::tcp::endpoint m_end_point;

  boost::asio::ip::tcp::acceptor m_acceptor;

  ServerImpl(const boost::asio::ip::address& addr,
             unsigned int port,
             Observer& o)
    : m_observer(o)
    , m_end_point(addr, port)
    , m_acceptor(m_io_context, m_end_point)
  {}

  void StartAccept();

  void HandleAccept(TcpConnection::Pointer new_connection,
                    const boost::system::error_code& ec);
};

void
ServerImpl::StartAccept()
{
  TcpConnection::Pointer new_connection =
    TcpConnection::Create(m_observer, m_io_context);

  m_acceptor.async_accept(new_connection->GetSocket(),
                          boost::bind(&ServerImpl::HandleAccept,
                                      this,
                                      new_connection,
                                      boost::asio::placeholders::error));
}

void
ServerImpl::HandleAccept(TcpConnection::Pointer new_connection,
                         const boost::system::error_code& ec)
{
  if (!ec) {
    new_connection->Start();
  }

  StartAccept();
}

Server::Server(const char* addr, unsigned int port)
  : Server(addr, port, DefaultServerObserver::GetInstance())
{}

Server::Server(const char* addr_str, unsigned int port, ServerObserver& o)
  : m_impl(nullptr)
{
  boost::system::error_code ec;

  boost::asio::ip::address addr =
    boost::asio::ip::address::from_string(addr_str, ec);

  if (ec.value() != 0) {
    o.OnAddressParseFailure(addr_str);
    return;
  }

  m_impl = new ServerImpl(addr, port, o);
}

Server::Server(Server&& other)
  : m_impl(other.m_impl)
{
  other.m_impl = nullptr;
}

Server::~Server()
{
  delete m_impl;
}

bool
Server::Run()
{
  if (!m_impl)
    return false;

  m_impl->StartAccept();

  m_impl->m_io_context.run();

  return true;
}

bool
Server::CanRun() const
{
  return m_impl ? true : false;
}

} // namespace vision
