#pragma once

#include <QTabWidget>

#include <stddef.h>

class QString;

namespace vision::gui {

class Monitor : public QTabWidget
{
public:
  Monitor(QWidget* parent)
    : QTabWidget(parent)
  {}

  virtual ~Monitor() = default;

  virtual void LogError(const QString& msg) = 0;

  virtual void LogInfo(const QString& msg) = 0;

  virtual void LogConnectionWrite(size_t byte_count) = 0;

  virtual void LogConnectionRead(size_t byte_count) = 0;
};

Monitor*
CreateMonitor(QWidget* parent);

} // namespace vision::gui
