#include "process_view.hpp"

#include "schedule.hpp"

#include <QString>
#include <QStringList>
#include <QWidget>

#include <sstream>

#include <QDebug>

namespace vision::gui {

namespace {

void
FlushAsLine(std::ostringstream& stream, QProcess& process)
{
  stream << '\n';

  const std::string data = stream.str();

  process.write(&data[0], data.size());
}

} // namespace

ProcessView::ProcessView(QWidget* parent, const QString& program_path)
  : ContentView(parent)
  , m_process(this)
{
  m_process.setProgram(program_path);

  QStringList arguments;

  // TODO : allow user to pass arguments.

  m_process.setArguments(arguments);

  // Setup other widgets

  GetLayout()->addWidget(&m_tool_tabs);

  m_tool_tabs.addTab(&m_stderr_log, tr("Error Log"));

  GetView()->SetObserver(this);

  // Setup signals/slots

  connect(&m_process,
          &QProcess::readyReadStandardError,
          this,
          &ProcessView::ForwardStandardErrorToLog);

  connect(&m_process,
          &QProcess::readyReadStandardOutput,
          this,
          &ProcessView::ReadResponse);

  connect(&m_process, &QProcess::started, this, &ProcessView::BeginRendering);
}

void
ProcessView::PrepareToClose()
{
  if (m_process.state() != QProcess::NotRunning) {
    m_process.write("q\n", 2);

    if (!m_process.waitForFinished(1000))
      m_process.kill();
  }
}

void
ProcessView::ForwardStandardErrorToLog()
{
  QByteArray data = m_process.readAllStandardError();

  m_stderr_log.insertPlainText(QString(data));
}

void
ProcessView::ReadResponse()
{
  QByteArray data = m_process.readAllStandardOutput();

  HandleResponse(data);
}

void
ProcessView::BeginRendering()
{
  GetView()->NewFrame();
}

void
ProcessView::OnError()
{
  m_process.kill();
}

void
ProcessView::OnKeyEvent(const QString& key, bool state)
{
  std::ostringstream stream;

  stream << "k " << key.toStdString() << ' ' << int(state);

  FlushAsLine(stream, m_process);
}

void
ProcessView::OnMouseButtonEvent(const QString& button, int x, int y, bool state)
{
  std::ostringstream stream;

  stream << "b " << button.toStdString() << ' ' << x << ' ' << y << ' '
         << int(state);

  FlushAsLine(stream, m_process);
}

void
ProcessView::OnMouseMoveEvent(int x, int y)
{
  std::ostringstream stream;

  stream << "m " << x << ' ' << y;

  FlushAsLine(stream, m_process);
}

void
ProcessView::OnNewFrame(const Schedule& schedule)
{
  const size_t req_count = schedule.GetRenderRequestCount();

  std::ostringstream stream;

  for (size_t i = 0; i < req_count; i++) {

    const RenderRequest req = schedule.GetRenderRequest(i);

    stream << "r " << req.x_pixel_count << ' ' << req.y_pixel_count;

    stream << '\n';
  }

  const std::string data = stream.str();

  m_process.write(&data[0], data.size());
}

void
ProcessView::OnResize(size_t w, size_t h, size_t padded_w, size_t padded_h)
{
  std::ostringstream stream;

  stream << "s " << w << ' ' << h << ' ' << padded_w << ' ' << padded_h;

  FlushAsLine(stream, m_process);
}

} // namespace vision::gui
