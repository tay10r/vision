#pragma once

#include <QWidget>

class QIODevice;
class QString;

namespace vision::gui {

class ContentViewImpl;

class ContentView : public QWidget
{
  Q_OBJECT
public:
  ContentView(QWidget* parent, QIODevice* io_device);

  virtual ~ContentView();

  QIODevice* GetIODevice() noexcept;

  void BeginRendering();

  void SendQuitCommand();

  virtual void ForceQuit() = 0;

signals:
  void InvalidResponse(const QString& reason);

  void BufferOverflow(size_t buffer_max);

protected:
  void HandleIncomingData(const QByteArray&);

protected slots:
  void ReadIODevice();

  void ForwardRGBBuffer(const unsigned char* rgb_buffer,
                        size_t w,
                        size_t h,
                        size_t req_id);

protected:
  void AddToolTab(const QString& name, QWidget* widget);

private:
  ContentViewImpl* m_impl;
};

ContentView*
CreateContentView(QWidget* parent, QIODevice* io_device);

} // namespace vision::gui
