#pragma once

#ifndef VISION_SERVER_HPP
#define VISION_SERVER_HPP

namespace vision {

class ServerObserver;
class ServerImpl;

class Server final
{
public:
  Server(const char* addr = "127.0.0.1", unsigned int port = 2194);

  Server(const char* addr, unsigned int port, ServerObserver&);

  Server(Server&& other);

  ~Server();

  /// @return True if the server can be started with "Run", or false if
  /// initialization failed.
  bool CanRun() const;

  /// Starts accepting connections.
  ///
  /// @return True on success, false on failure.
  bool Run();

private:
  ServerImpl* m_impl;
};

} // namespace vision

#endif // VISION_SERVER_HPP
