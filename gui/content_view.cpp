#include "content_view.hpp"

#include "command_stream.hpp"
#include "render_request.hpp"
#include "resize_request.hpp"
#include "response.hpp"
#include "response_signal_emitter.hpp"
#include "view.hpp"

#include <QTabWidget>
#include <QVBoxLayout>

namespace vision::gui {

namespace {

class ViewEventStreamer final : public ViewObserver
{
public:
  ViewEventStreamer(QIODevice& io_device)
    : m_command_stream(io_device)
  {}

  void SetEnabled(bool enabled) { m_enabled = enabled; }

  void OnNewFrame(const Schedule& schedule) override
  {
    if (m_enabled)
      m_command_stream.SendAllRenderRequests(schedule);
  }

  void OnResize(size_t w, size_t h, size_t padded_w, size_t padded_h) override
  {
    if (m_enabled) {

      const ResizeRequest req = { w, h, padded_w, padded_h };

      m_command_stream.SendResizeRequest(req);
    }
  }

  void OnKeyEvent(const QString& key, bool state) override
  {
    if (m_enabled)
      m_command_stream.SendKey(key, state);
  }

  void OnMouseButtonEvent(const QString& button,
                          int x,
                          int y,
                          bool state) override
  {
    if (m_enabled)
      m_command_stream.SendMouseButton(button, x, y, state);
  }

  void OnMouseMoveEvent(int x, int y) override
  {
    if (m_enabled)
      m_command_stream.SendMouseMove(x, y);
  }

private:
  CommandStream m_command_stream;

  bool m_enabled = false;
};

} // namespace

class ContentViewImpl final
{
  friend ContentView;

  ContentViewImpl(ContentView* self, QIODevice* io_device)
    : m_io_device(io_device)
    , m_view(CreateView(self))
    , m_layout(self)
    , m_tool_tabs(self)
    , m_response_signal_emitter(self)
    , m_view_event_streamer(*io_device)
  {
    m_layout.addWidget(m_view);

    m_layout.addWidget(&m_tool_tabs);
  }

  QIODevice* m_io_device;

  View* m_view;

  QVBoxLayout m_layout;

  QTabWidget m_tool_tabs;

  ResponseSignalEmitter m_response_signal_emitter;

  ViewEventStreamer m_view_event_streamer;

  std::unique_ptr<ResponseParser> m_response_parser =
    ResponseParser::Create(m_response_signal_emitter);
};

ContentView::ContentView(QWidget* parent, QIODevice* io_device)
  : QWidget(parent)
  , m_impl(new ContentViewImpl(this, io_device))
{
  connect(&m_impl->m_response_signal_emitter,
          &ResponseSignalEmitter::InvalidResponse,
          this,
          &ContentView::InvalidResponse);

  connect(&m_impl->m_response_signal_emitter,
          &ResponseSignalEmitter::BufferOverflow,
          this,
          &ContentView::BufferOverflow);

  connect(&m_impl->m_response_signal_emitter,
          &ResponseSignalEmitter::RGBBuffer,
          this,
          &ContentView::ForwardRGBBuffer);

  m_impl->m_view->SetObserver(&m_impl->m_view_event_streamer);
}

ContentView::~ContentView()
{
  delete m_impl;
}

QIODevice*
ContentView::GetIODevice() noexcept
{
  return m_impl->m_io_device;
}

void
ContentView::BeginRendering()
{
  m_impl->m_view_event_streamer.SetEnabled(true);

  const ResizeRequest req = m_impl->m_view->MakeResizeRequest();

  CommandStream command_stream(*GetIODevice());

  command_stream.SendResizeRequest(req);

  m_impl->m_view->NewFrame();
}

void
ContentView::SendQuitCommand()
{
  CommandStream command_stream(*GetIODevice());

  command_stream.SendQuit();
}

void
ContentView::HandleIncomingData(const QByteArray& data)
{
  m_impl->m_response_parser->Write(data.constData(), data.size());
}

void
ContentView::ReadIODevice()
{
  const QByteArray data = GetIODevice()->readAll();

  HandleIncomingData(data);
}

void
ContentView::ForwardRGBBuffer(const unsigned char* buffer,
                              size_t w,
                              size_t h,
                              size_t req_id)
{
  m_impl->m_view->ReplyRenderRequest(buffer, w * h * 3, req_id);
}

void
ContentView::AddToolTab(const QString& name, QWidget* widget)
{
  m_impl->m_tool_tabs.addTab(widget, name);
}

} // namespace vision::gui
