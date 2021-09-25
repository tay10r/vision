#pragma once

#include "content_view.hpp"

class QProcess;
class QByteArray;

namespace vision::gui {

class ProcessViewImpl;

class ProcessView : public ContentView
{
  Q_OBJECT
public:
  ProcessView(QWidget* parent, const QString& program_path);

  ~ProcessView();

  QProcess* GetProcess();

  void StartProcess();

  void ForceQuit() override;

protected slots:
  void ReadStandardOutput();

  void ReadStandardError();

private:
  ProcessView(QWidget* parent, QProcess* process);

private:
  ProcessViewImpl* m_impl;
};

} // namespace vision::gui
