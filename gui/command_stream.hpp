#pragma once

#include <iosfwd>

class QIODevice;
class QString;

namespace vision::gui {

struct RenderRequest;
struct ResizeRequest;

class Schedule;

class CommandStream final
{
public:
  CommandStream(QIODevice& io_device)
    : m_io_device(io_device)
  {}

  void SendAllRenderRequests(const Schedule&);

  void SendRenderRequest(const RenderRequest&);

  void SendResizeRequest(const ResizeRequest&);

  void SendKey(const QString& key, bool state);

  void SendMouseButton(const QString& button, int x, int y, bool state);

  void SendMouseMove(int x, int y);

  void SendQuit();

protected:
  void Write(std::ostream& output_stream, const RenderRequest&);

private:
  QIODevice& m_io_device;
};

} // namespace vision::gui
