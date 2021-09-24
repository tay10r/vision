#pragma once

#include "response.hpp"
#include "response_signal_emitter.hpp"

#include <QByteArray>
#include <QVBoxLayout>
#include <QWidget>

class QIODevice;

namespace vision::gui {

class View;

class ContentView : public QWidget
{
  Q_OBJECT
public:
  ContentView(QWidget* parent);

  ContentView(const ContentView&) = delete;

  virtual void PrepareToClose() = 0;

  ResponseSignalEmitter* GetResponseSignalEmitter()
  {
    return &m_response_signal_emitter;
  }

protected:
  View* GetView() noexcept { return m_view; }

  QVBoxLayout* GetLayout() noexcept { return &m_layout; }

  void HandleResponse(const QByteArray&);

  void OnRGBBuffer(const unsigned char* rgb_buffer,
                   size_t w,
                   size_t h,
                   size_t request_id);

private:
  View* m_view;

  QVBoxLayout m_layout{ this };

  ResponseSignalEmitter m_response_signal_emitter{ this };

  std::unique_ptr<ResponseParser> m_response_parser =
    ResponseParser::Create(m_response_signal_emitter);
};

} // namespace vision::gui
