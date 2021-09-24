#pragma once

#include "content_view.hpp"
#include "view.hpp"

#include <QProcess>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

namespace vision::gui {

class ProcessView final
  : public ContentView
  , public ViewObserver
{
public:
  ProcessView(QWidget* parent, const QString& program_path);

  QProcess* GetProcess() noexcept { return &m_process; }

protected slots:
  void ForwardStandardErrorToLog();

  void ReadResponse();

  void OnError();

  void BeginRendering();

protected:
  void OnKeyEvent(const QString& key, bool state) override;

  void OnMouseButtonEvent(const QString& button,
                          int x,
                          int y,
                          bool state) override;

  void OnMouseMoveEvent(int x, int y) override;

  void OnNewFrame(const Schedule&) override;

  void OnResize(size_t w, size_t h, size_t padded_w, size_t padded_h) override;

private:
  QProcess m_process;

  QWidget* m_view;

  QTabWidget m_tool_tabs{ this };

  QTextEdit m_stderr_log{ &m_tool_tabs };
};

} // namespace vision::gui
