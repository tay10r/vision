#pragma once

#include <string>
#include <vector>

#include <stddef.h>

namespace vision {

class ServerImpl;
class Function;

class Server final
{
public:
  Server();

  Server(Server&&);

  ~Server();

  /// Defines a function.
  ///
  /// @param name The name of the function to define.
  ///
  /// @param function A pointer to the function. The server takes ownership of
  /// the function.
  void DefineFunction(const char* name, Function* function);

  /// Handles command data sent by the browser. The command data does not have
  /// to be complete.
  ///
  /// @param data The command data string.
  ///
  /// @param size The number of characters in the string.
  ///
  /// @return If the server receives a full command queue, the contents of the
  ///         render buffer are returned by this function.
  auto Process(const char* data, size_t size) -> std::vector<char>;

  auto ReadOutStream() -> std::string;

  auto ReadErrStream() -> std::string;

private:
  ServerImpl* m_impl;

  ServerImpl& GetImpl();
};

} // namespace vision
