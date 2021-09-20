#pragma once

#include <stddef.h>

namespace vision::gui {

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

  virtual void Resize(size_t w, size_t h) = 0;

  virtual void Render(size_t w,
                      size_t h,
                      size_t x_offset,
                      size_t y_offset,
                      size_t x_stride,
                      size_t y_stride) = 0;
};

} // namespace vision::gui
