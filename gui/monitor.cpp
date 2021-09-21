#include "monitor.hpp"

#include <QChart>
#include <QChartView>
#include <QSplineSeries>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>

#include <algorithm>
#include <chrono>

#include <iostream>

namespace vision::gui {

namespace {

class MonitorImpl final : public Monitor
{
public:
  using Clock = std::chrono::high_resolution_clock;

  using TimePoint = Clock::time_point;

  using Microseconds = std::chrono::microseconds;

  MonitorImpl(QWidget* parent)
    : Monitor(parent)
    , m_start_time(Clock::now())
  {
    addTab(&m_log, "Log");

    addTab(&m_io_chart_view, "IO");

    m_io_chart_view.setRenderHint(QPainter::Antialiasing);

    m_io_chart_view.setChart(m_io_chart);

    m_io_chart->addSeries(m_read_series);

    m_io_chart->createDefaultAxes();

    m_log.setReadOnly(true);

    m_sampling_timer.setInterval(m_sampling_interval);

    connect(&m_sampling_timer,
            &QTimer::timeout,
            this,
            &MonitorImpl::HandleSamplingPeriod);

    m_sampling_timer.start();
  }

  ~MonitorImpl()
  {
    delete m_read_series;

    delete m_io_chart;
  }

  void LogError(const QString& msg) override
  {
    QColor original_color = m_log.textColor();

    m_log.setTextColor(QColor("Red"));

    m_log.insertPlainText("ERROR: ");

    m_log.setTextColor(original_color);

    m_log.insertPlainText(msg);

    m_log.insertPlainText("\n");
  }

  void LogInfo(const QString& msg) override
  {
    m_log.insertPlainText(msg);

    m_log.insertPlainText("\n");
  }

  void LogConnectionWrite(size_t byte_count) override { (void)byte_count; }

  void LogConnectionRead(size_t byte_count) override
  {
    m_read_count += byte_count;
  }

private:
  void HandleSamplingPeriod()
  {
    const float interval_in_seconds = m_sampling_interval / 1000.0f;

    const float read_speed = float(m_read_count) / interval_in_seconds;

    AddReadSpeedSample(read_speed);

    m_read_count = 0;
  }

  void AddReadSpeedSample(float read_speed)
  {
    TimePoint t = Clock::now();

    size_t delta_t =
      std::chrono::duration_cast<Microseconds>(t - m_start_time).count();

    if (m_read_series->count() >= m_max_read_samples)
      m_read_series->remove(0);

    m_read_series->append(1.0e-6f * delta_t, read_speed);

    QVector<QPointF> points = m_read_series->pointsVector();

    if (points.empty())
      return;

    float min_delta_t = points.front().x();
    float max_delta_t = points.back().x();

    float min_read_speed = points[0].y();
    float max_read_speed = points[0].y();

    for (const auto& p : points) {
      min_read_speed = std::min(min_read_speed, float(p.y()));
      max_read_speed = std::max(max_read_speed, float(p.y()));
    }

    QList<QtCharts::QAbstractAxis*> axes = m_io_chart->axes();

    axes[0]->setMin(min_delta_t);
    axes[0]->setMax(max_delta_t);

    axes[1]->setMin(min_read_speed);
    axes[1]->setMax(max_read_speed);
  }

private:
  QTimer m_sampling_timer;

  int m_sampling_interval = 100;

  size_t m_read_count = 0;

  int m_max_read_samples = 128;

  TimePoint m_start_time;

  QTextEdit m_log{ this };

  QtCharts::QChart* m_io_chart = new QtCharts::QChart();

  QtCharts::QSplineSeries* m_read_series = new QtCharts::QSplineSeries();

  QtCharts::QChartView m_io_chart_view{ this };
};

} // namespace

Monitor*
CreateMonitor(QWidget* parent)
{
  return new MonitorImpl(parent);
}

} // namespace vision::gui
