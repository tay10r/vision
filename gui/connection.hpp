#pragma once

#include <string_view>

#include <stddef.h>

namespace vision::gui {

struct RenderRequest;
struct ResizeRequest;

class ConnectionObserver
{
public:
  virtual ~ConnectionObserver() = default;

  /// Called when a connection is established.
  virtual void OnConnectionStart() = 0;

  /// Called whenever data is received from the connection.
  ///
  /// @param data A pointer to the beginning of the data.
  ///
  /// @param length The number of characters in the data packet.
  virtual void OnConnectionRecv(const unsigned char* data, size_t length) = 0;
};

class Connection
{
public:
  virtual ~Connection() = default;

  virtual bool Connect() = 0;

  virtual void Resize(const ResizeRequest&) = 0;

  virtual void Render(const RenderRequest&) = 0;

  virtual void SendKey(const std::string_view& key, bool state) = 0;

  virtual void SendMouseButton(const std::string_view& button_name,
                               int x,
                               int y,
                               bool state) = 0;

  virtual void SendMouseMove(int x, int y) = 0;
};

} // namespace vision::gui
