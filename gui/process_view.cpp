#include "process_view.hpp"

#include "command_stream.hpp"
#include "resize_request.hpp"
#include "schedule.hpp"

#include <QString>
#include <QStringList>
#include <QWidget>

#include <sstream>

#include <QDebug>

namespace vision::gui {

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

    qDebug() << "here: " << m_process.state();

    CommandStream command_stream(m_process);

    command_stream.SendQuit();

    if (!m_process.waitForFinished(1000)) {
      qDebug() << "killing process";
      m_process.kill();
      if (m_process.waitForFinished(1000))
        qDebug() << "process was killed";
      else
        qDebug() << "process was not killed.";
    } else {
      qDebug() << "process exited gracefully";
    }
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
  CommandStream command_stream(m_process);

  command_stream.SendResizeRequest(GetView()->MakeResizeRequest());

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
  CommandStream command_stream(m_process);

  command_stream.SendKey(key, state);
}

void
ProcessView::OnMouseButtonEvent(const QString& button, int x, int y, bool state)
{
  CommandStream command_stream(m_process);

  command_stream.SendMouseButton(button, x, y, state);
}

void
ProcessView::OnMouseMoveEvent(int x, int y)
{
  CommandStream command_stream(m_process);

  command_stream.SendMouseMove(x, y);
}

void
ProcessView::OnNewFrame(const Schedule& schedule)
{
  CommandStream command_stream(m_process);

  command_stream.SendAllRenderRequests(schedule);
}

void
ProcessView::OnResize(size_t w, size_t h, size_t padded_w, size_t padded_h)
{
  CommandStream command_stream(m_process);

  command_stream.SendResizeRequest(ResizeRequest{ w, h, padded_w, padded_h });
}

} // namespace vision::gui
