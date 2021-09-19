#include "monitor.hpp"

#include <QChart>
#include <QChartView>
#include <QTabWidget>
#include <QTextEdit>

namespace vision::gui {

namespace {

class MonitorImpl final : public Monitor
{
public:
  MonitorImpl(QWidget* parent)
    : Monitor(parent)
  {
    addTab(&m_log, "Log");

    addTab(&m_io_chart_view, "IO");

    m_io_chart_view.setChart(m_io_chart);

    m_log.setReadOnly(true);
  }

  ~MonitorImpl() { delete m_io_chart; }

  void LogError(const QString&) override {}

  void LogInfo(const QString&) override {}

  void LogConnectionWrite(size_t byte_count) override { (void)byte_count; }

  void LogConnectionRead(size_t byte_count) override { (void)byte_count; }

private:
  QTextEdit m_log{ this };

  QtCharts::QChart* m_io_chart = new QtCharts::QChart();

  QtCharts::QChartView m_io_chart_view{ this };
};

} // namespace

Monitor*
CreateMonitor(QWidget* parent)
{
  return new MonitorImpl(parent);
}

} // namespace vision::gui
