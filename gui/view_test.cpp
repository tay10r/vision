#include <QApplication>
#include <QMainWindow>
#include <QSurfaceFormat>
#include <QTimer>
#include <QVector2D>

#include "schedule.hpp"
#include "view.hpp"

#include <iostream>

namespace {

void
PrintRenderRequest(const vision::gui::RenderRequest& req,
                   std::ostream& out = std::cout)
{
  for (int x = 0; x < req.x_pixel_stride; x++)
    out << '=';

  out << std::endl;

  for (int y = 0; y < req.y_pixel_stride; y++) {

    for (int x = 0; x < req.x_pixel_stride; x++) {
      if ((x == req.x_pixel_offset) && (y == req.y_pixel_offset))
        out << '*';
      else
        out << 'O';
    }

    out << std::endl;
  }
}

struct Circle final
{
  float u;
  float v;
  float r;

  bool Contains(float other_u, float other_v) const
  {
    float a = u - other_u;
    float b = v - other_v;
    return ((a * a) + (b * b)) < (r * r);
  }
};

std::vector<unsigned char>
HandleRenderRequest(const vision::gui::RenderRequest& req)
{
  std::vector<unsigned char> buffer(req.x_pixel_count * req.y_pixel_count * 3);

  float u_scale = 1.0f / req.x_frame_size;
  float v_scale = 1.0f / req.y_frame_size;

  const Circle circle{ 0.5, 0.5, 0.1 };

  for (int y = 0; y < req.y_pixel_count; y++) {

    for (int x = 0; x < req.x_pixel_count; x++) {

      int xx = req.x_pixel_offset + (x * req.x_pixel_stride);
      int yy = req.y_pixel_offset + (y * req.y_pixel_stride);

      float u = (xx + 0.5f) / req.x_frame_size;
      float v = (yy + 0.5f) / req.y_frame_size;

      float k = circle.Contains(u, v) ? 1 : 0;

      int i = (y * req.x_pixel_count) + x;

      unsigned char* rgb = &buffer[i * 3];

      rgb[0] = u * k * 255;
      rgb[1] = v * k * 255;
      rgb[2] = k * 255;
    }
  }

  return buffer;
}

} // namespace

int
main(int argc, char** argv)
{
  QApplication app(argc, argv);

#if 0
  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);
#endif

  QMainWindow main_window;

  main_window.resize(1280, 720);

  main_window.show();

  vision::gui::View* view = vision::gui::CreateView(&main_window);

  main_window.setCentralWidget(view);

  view->SetDivisionLevel(2);

  QTimer timer(&main_window);

  QObject::connect(&timer, &QTimer::timeout, [&timer, view] {
    const vision::gui::RenderRequest req = view->GetCurrentRenderRequest();

    if (!req.IsValid())
      return;

    PrintRenderRequest(req);

    std::vector<unsigned char> buf = HandleRenderRequest(req);

    view->ReplyRenderRequest(buf.data(), buf.size());
  });

  timer.start(50);

  return app.exec();
}
