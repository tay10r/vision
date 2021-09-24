#include "command_stream.hpp"

#include "render_request.hpp"
#include "resize_request.hpp"
#include "schedule.hpp"

#include <sstream>

#include <QIODevice>
#include <QString>

namespace vision::gui {

namespace {

void
Flush(std::ostringstream& stream, QIODevice& io_device)
{
  const std::string data = stream.str();

  io_device.write(&data[0], data.size());
}

} // namespace

void
CommandStream::SendAllRenderRequests(const Schedule& schedule)
{
  std::ostringstream stream;

  const size_t req_count = schedule.GetRenderRequestCount();

  for (size_t i = 0; i < req_count; i++) {

    const RenderRequest req = schedule.GetRenderRequest(i);

    Write(stream, req);
  }

  Flush(stream, m_io_device);
}

void
CommandStream::SendRenderRequest(const RenderRequest& req)
{
  std::ostringstream stream;

  Write(stream, req);

  Flush(stream, m_io_device);
}

void
CommandStream::Write(std::ostream& output, const RenderRequest& req)
{
  output << "r ";
  output << req.x_pixel_count << ' ' << req.y_pixel_count;
  output << ' ';
  output << req.x_pixel_offset << ' ' << req.y_pixel_offset;
  output << ' ';
  output << req.y_pixel_stride << ' ' << req.y_pixel_stride;
  output << ' ';
  output << req.id << '\n';
}

void
CommandStream::SendResizeRequest(const ResizeRequest& req)
{
  std::ostringstream stream;

  stream << "s ";

  stream << req.width << ' ' << req.height;

  stream << req.padded_width << ' ' << req.padded_height;

  stream << '\n';

  Flush(stream, m_io_device);
}

void
CommandStream::SendKey(const QString& key, bool state)
{
  std::ostringstream stream;

  stream << "k " << key.toStdString() << " " << int(state);

  stream << '\n';

  Flush(stream, m_io_device);
}

void
CommandStream::SendMouseButton(const QString& button, int x, int y, bool state)
{
  std::ostringstream stream;

  stream << "b ";
  stream << button.toStdString();
  stream << ' ';
  stream << x << ' ' << y;
  stream << ' ' << int(state);
  stream << '\n';

  Flush(stream, m_io_device);
}

void
CommandStream::SendMouseMove(int x, int y)
{
  std::ostringstream stream;

  stream << "m ";
  stream << x << ' ' << y;
  stream << '\n';

  Flush(stream, m_io_device);
}

void
CommandStream::SendQuit()
{
  std::ostringstream stream;

  stream << "q\n";

  Flush(stream, m_io_device);
}

} // namespace vision::gui
