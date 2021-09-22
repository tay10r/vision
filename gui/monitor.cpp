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

    m_io_chart->legend()->hide();

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

    if (m_samples.size() >= m_max_read_samples)
      m_samples.erase(m_samples.begin());

    m_samples.emplace_back(IOSample{ float(delta_t), read_speed });

    m_read_series->append(1.0e-6f * delta_t, read_speed);

    if (m_samples.empty())
      return;

    float min_delta_t = m_samples.front().time;
    float max_delta_t = m_samples.back().time;

    float min_read_speed = m_samples[0].speed;
    float max_read_speed = m_samples[0].speed;

    for (const auto& sample : m_samples) {
      min_read_speed = std::min(min_read_speed, sample.speed);
      max_read_speed = std::max(max_read_speed, sample.speed);
    }

    min_delta_t *= 1e-6;
    max_delta_t *= 1e-6;

    const char* units = "B";

    float speed_div = 1;

    if (max_read_speed >= 1073741824.0f) {
      speed_div = 1073741824.0f;
      units = "GiB";
    } else if (max_read_speed >= 1048576.0f) {
      speed_div = 1048576.0f;
      units = "MiB";
    } else if (max_read_speed >= 1024.0f) {
      speed_div = 1024.0f;
      units = "KiB";
    }

    QVector<QPointF> points;

    points.reserve(m_samples.size());

    for (const auto& sample : m_samples) {

      const QPointF point(sample.time * 1e-6, sample.speed / speed_div);

      points.push_back(point);
    }

    m_read_series->replace(points);

    QList<QtCharts::QAbstractAxis*> axes = m_io_chart->axes();

    axes[0]->setMin(min_delta_t);
    axes[0]->setMax(max_delta_t);

    axes[1]->setMin(min_read_speed / speed_div);
    axes[1]->setMax(max_read_speed / speed_div);
    axes[1]->setTitleText(QString("Read Speed (%1/s)").arg(units));
  }

private:
  struct IOSample final
  {
    float time;
    float speed;
  };

  QTimer m_sampling_timer;

  int m_sampling_interval = 100;

  size_t m_read_count = 0;

  size_t m_max_read_samples = 128;

  std::vector<IOSample> m_samples;

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
