#include "response_signal_emitter.hpp"

namespace vision::gui {

void
ResponseSignalEmitter::OnRGBBuffer(const unsigned char* rgb_buffer,
                                   size_t w,
                                   size_t h,
                                   size_t req_id)
{
  emit RGBBuffer(rgb_buffer, w, h, req_id);
}

void
ResponseSignalEmitter::OnBufferOverflow(size_t buffer_max)
{
  emit BufferOverflow(buffer_max);
}

void
ResponseSignalEmitter::OnInvalidResponse(const std::string_view& reason)
{
  emit InvalidResponse(QString::fromUtf8(reason.data(), reason.size()));
}

} // namespace vision::gui
