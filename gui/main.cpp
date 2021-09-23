#include <QApplication>
#include <QMainWindow>
#include <QSurfaceFormat>

#include "page.hpp"

namespace {

class MainWindow : public QMainWindow
{
public:
  MainWindow()
    : m_central_widget(vision::gui::CreatePage(this))
  {
    resize(1280, 720);

    setWindowTitle("Vision");

    setCentralWidget(m_central_widget);
  }

private:
  QWidget* m_central_widget;
};

} // namespace

int
main(int argc, char** argv)
{
  QApplication app(argc, argv);

  Q_INIT_RESOURCE(shaders);

#if 0
  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setVersion(3, 2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);
#endif

  MainWindow main_window;

  main_window.show();

  return app.exec();
}
