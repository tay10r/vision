#include "content_view.hpp"

#include "view.hpp"

namespace vision::gui {

ContentView::ContentView(QWidget* parent)
  : QWidget(parent)
  , m_view(CreateView(this))
{
  m_layout.addWidget(m_view);
}

void
ContentView::HandleResponse(const QByteArray& data)
{
  m_response_parser->Write(data.constData(), data.size());
}

void
ContentView::OnBufferOverflow()
{
  const size_t buffer_max = m_response_parser->GetMaxBufferSize();

  emit BufferOverflow(buffer_max);

  OnError();
}

void
ContentView::OnInvalidResponse(const std::string_view& reason)
{
  const QString reason_copy(QString::fromUtf8(reason.data(), reason.size()));

  emit InvalidResponse(reason_copy);

  OnError();
}

void
ContentView::OnRGBBuffer(const unsigned char* rgb_buffer,
                         size_t w,
                         size_t h,
                         size_t request_id)
{
  m_view->ReplyRenderRequest(rgb_buffer, w * h * 3, request_id);
}

} // namespace vision::gui
