#pragma once

#include <memory>
#include <string_view>

#include <stddef.h>

namespace vision::gui {

class ResponseObserver
{
public:
  virtual ~ResponseObserver() = default;

  virtual void OnInvalidResponse(const std::string_view& reason) = 0;

  /// This is called when the buffer reaches a size that is considered too large
  /// to be a valid response.
  virtual void OnBufferOverflow() = 0;

  virtual void OnRGBBuffer(const unsigned char* buffer,
                           size_t width,
                           size_t height) = 0;
};

class ResponseParser
{
public:
  static std::unique_ptr<ResponseParser> Create(ResponseObserver&);

  virtual ~ResponseParser() = default;

  /// @return True on success, false of the maximum buffer size is exceeded.
  virtual bool Write(const char* data, size_t length) = 0;

  virtual void SetMaxBufferSize(size_t max_size) = 0;
};

} // namespace vision::gui
