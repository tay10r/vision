#include <vision/server.hpp>

#include <vision/interpreter.hpp>
#include <vision/line_buffer.hpp>

#include <sstream>

namespace vision {

class ServerImpl final
{
  friend Server;

  std::ostringstream err_stream;

  std::ostringstream out_stream;

  LineBuffer line_buffer;

  Interpreter interpreter{ out_stream, err_stream };
};

Server::Server()
  : m_impl(new ServerImpl())
{}

Server::Server(Server&& other)
  : m_impl(other.m_impl)
{
  other.m_impl = nullptr;
}

Server::~Server()
{
  delete m_impl;
}

std::string
Server::ReadOutStream()
{
  auto out = GetImpl().out_stream.str();

  GetImpl().out_stream = std::ostringstream();

  return out;
}

std::string
Server::ReadErrStream()
{
  auto err = GetImpl().err_stream.str();

  GetImpl().err_stream = std::ostringstream();

  return err;
}

void
Server::DefineFunction(const char* name, Function* function)
{
  GetImpl().interpreter.SetFunc(name, function);
}

auto
Server::Process(const char* data, size_t size) -> std::vector<char>
{
  auto& impl = GetImpl();

  impl.line_buffer.Write(data, size);

  if (!impl.line_buffer.IsTerminated())
    return std::vector<char>();

  std::vector<char> buffer;

  return buffer;
}

ServerImpl&
Server::GetImpl()
{
  if (!m_impl)
    m_impl = new ServerImpl();

  return *m_impl;
}

} // namespace vision
