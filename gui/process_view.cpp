#include "process_view.hpp"

#include <QProcess>
#include <QTextEdit>

namespace vision::gui {

class ProcessViewImpl final
{
  friend ProcessView;

  ProcessViewImpl(QWidget* parent, QProcess* process)
    : m_process(process)
    , m_stderr_log(new QTextEdit(parent))
  {}

  QProcess* m_process;

  QTextEdit* m_stderr_log;
};

ProcessView::ProcessView(QWidget* parent, const QString& program_path)
  : ProcessView(parent, new QProcess(parent))
{
  m_impl->m_process->setProgram(program_path);
}

ProcessView::ProcessView(QWidget* parent, QProcess* process)
  : ContentView(parent, process)
  , m_impl(new ProcessViewImpl(this, process))
{
  connect(process,
          &QProcess::readyReadStandardOutput,
          this,
          &ProcessView::ReadStandardOutput);

  connect(process,
          &QProcess::readyReadStandardError,
          this,
          &ProcessView::ReadStandardError);

  AddToolTab("Error Log", m_impl->m_stderr_log);

  m_impl->m_stderr_log->setReadOnly(true);
}

ProcessView::~ProcessView()
{
  delete m_impl;
}

QProcess*
ProcessView::GetProcess()
{
  return m_impl->m_process;
}

void
ProcessView::ForceQuit()
{
  m_impl->m_process->kill();

  if (m_impl->m_process->state() != QProcess::NotRunning)
    m_impl->m_process->waitForFinished(1000);
}

void
ProcessView::ReadStandardOutput()
{
  const QByteArray data = m_impl->m_process->readAllStandardOutput();

  HandleIncomingData(data);
}

void
ProcessView::ReadStandardError()
{
  const QByteArray data = m_impl->m_process->readAllStandardError();

  m_impl->m_stderr_log->insertPlainText(QString::fromUtf8(data));
}

} // namespace vision::gui
