#pragma once

#include "response.hpp"

#include <QByteArray>
#include <QVBoxLayout>
#include <QWidget>

class QWidget;

namespace vision::gui {

class View;

class ContentView
  : public QWidget
  , public ResponseObserver
{
  Q_OBJECT
public:
  ContentView(QWidget* parent);

  ContentView(const ContentView&) = delete;

signals:
  void BufferOverflow(size_t limit);

  void InvalidResponse(const QString& reason);

protected:
  View* GetView() noexcept { return m_view; }

  QVBoxLayout* GetLayout() noexcept { return &m_layout; }

  void HandleResponse(const QByteArray&);

  void OnBufferOverflow() override;

  void OnRGBBuffer(const unsigned char* rgb_buffer,
                   size_t w,
                   size_t h,
                   size_t request_id) override;

  void OnInvalidResponse(const std::string_view& reason) override;

  virtual void OnError() = 0;

private:
  View* m_view;

  QVBoxLayout m_layout{ this };

  std::unique_ptr<ResponseParser> m_response_parser =
    ResponseParser::Create(*this);
};

} // namespace vision::gui
