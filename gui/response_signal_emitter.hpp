#pragma once

#include "response.hpp"

#include <QObject>

namespace vision::gui {

class ResponseSignalEmitter final
  : public QObject
  , public ResponseObserver
{
  Q_OBJECT
public:
  ResponseSignalEmitter(QObject* parent)
    : QObject(parent)
  {}

signals:
  void RGBBuffer(const unsigned char* rgb, size_t w, size_t h, size_t req_id);

  void BufferOverflow(size_t buffer_max);

  void InvalidResponse(const QString& reason);

protected:
  void OnRGBBuffer(const unsigned char*, size_t, size_t, size_t) override;

  void OnBufferOverflow(size_t buffer_max) override;

  void OnInvalidResponse(const std::string_view& reason) override;
};

} // namespace vision::gui
