#include "page.hpp"

#include "address_bar.hpp"
#include "content_view.hpp"
#include "process_view.hpp"
#include "view.hpp"

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace vision::gui {

namespace {

class InitialContentView final : public QLabel
{
public:
  InitialContentView(QWidget* parent)
    : QLabel(parent)
  {
    setText(tr("Connect to a renderer with the navigation bar above."));

    setAlignment(Qt::AlignCenter);
  }
};

class ContentArea : public QStackedWidget
{
public:
  ContentArea(QWidget* parent)
    : QStackedWidget(parent)
  {
    addWidget(&m_initial_content_view);
  }

  void SetContentView(ContentView* content_view)
  {
    if (m_content_view) {
      removeWidget(m_content_view);
      delete m_content_view;
    }

    m_content_view = content_view;

    if (content_view) {
      const int index = addWidget(m_content_view);
      setCurrentIndex(index);
    }

    RemoveErrorView();
  }

  void SetErrorView(QWidget* error_view)
  {
    RemoveErrorView();

    m_error_view = error_view;

    if (m_error_view) {
      const int index = addWidget(m_error_view);
      setCurrentIndex(index);
    }
  }

  void PrepareToClose()
  {
    if (m_content_view)
      m_content_view->PrepareToClose();
  }

private:
  void RemoveErrorView()
  {
    if (m_error_view) {

      removeWidget(m_error_view);

      delete m_error_view;

      m_error_view = nullptr;
    }
  }

private:
  InitialContentView m_initial_content_view{ this };

  ContentView* m_content_view = nullptr;

  QWidget* m_error_view = nullptr;
};

class PageImpl final : public Page
{
public:
  PageImpl(QWidget* parent)
    : Page(parent)
  {
    m_layout.addWidget(&m_address_bar);

    m_layout.addWidget(&m_content_area, 1);

    connect(&m_address_bar,
            &AddressBar::ConnectionRequest,
            this,
            &PageImpl::OnConnectionRequest);
  }

  void PrepareToClose() override { m_content_area.PrepareToClose(); }

protected slots:
  void OnConnectionRequest(const Address& address)
  {
    m_content_area.SetContentView(nullptr);

    switch (address.kind) {
      case AddressKind::Debug:
        break;
      case AddressKind::File:
        StartProgram(address.data);
        break;
      case AddressKind::Tcp:
        break;
      case AddressKind::Unknown:
        break;
    }
  }

private:
  void StartProgram(const QString& path)
  {
    ProcessView* process_view = new ProcessView(&m_content_area, path);

    QProcess* process = process_view->GetProcess();

    connect(process, &QProcess::errorOccurred, this, &PageImpl::OnProcessError);

    ResponseSignalEmitter* response_signal_emitter =
      process_view->GetResponseSignalEmitter();

    connect(response_signal_emitter,
            &ResponseSignalEmitter::BufferOverflow,
            this,
            &PageImpl::OnBufferOverflow);

    connect(response_signal_emitter,
            &ResponseSignalEmitter::InvalidResponse,
            this,
            &PageImpl::OnInvalidResponse);

    m_content_area.SetContentView(process_view);

    process->start();
  }

  void OnProcessError(QProcess::ProcessError error)
  {
    switch (error) {
      case QProcess::FailedToStart:
        EmitError("Failed to start process.");
        break;
      case QProcess::Crashed:
        EmitError("Process crashed.");
        break;
      case QProcess::Timedout:
        EmitError("Process timed out.");
        break;
      case QProcess::WriteError:
        EmitError("Write error occurred.");
        break;
      case QProcess::ReadError:
        EmitError("Read error occurred.");
        break;
      case QProcess::UnknownError:
        EmitError("Unknown error occurred.");
        break;
    }
  }

  void OnBufferOverflow(size_t buffer_max)
  {
    EmitError(QString("Buffer size exceeded %1").arg(buffer_max));
  }

  void OnInvalidResponse(const QString& reason)
  {
    EmitError(QString("Invalid Response: ") + reason);
  }

  void EmitError(const QString& msg)
  {
    QLabel* label = new QLabel(msg, &m_content_area);

    label->setAlignment(Qt::AlignCenter);

    m_content_area.SetErrorView(label);
  }

  void OnMonitorVisibilityToggle(bool visible)
  {
    // m_monitor->setVisible(visible);

    (void)visible;
  }

  void DisconnectDueToError()
  {
#if 0
    m_monitor->LogInfo("Disconnecting due to error.");

    ResetConnection();
#endif
  }

private:
  AddressBar m_address_bar{ this };

  ContentArea m_content_area{ this };

  QVBoxLayout m_layout{ this };
};

} // namespace

Page*
CreatePage(QWidget* parent)
{
  return new PageImpl(parent);
}

} // namespace vision::gui
