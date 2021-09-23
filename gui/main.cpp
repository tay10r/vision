#include <QApplication>
#include <QMainWindow>
#include <QSurfaceFormat>

#include "controller.hpp"
#include "page.hpp"

namespace {

class MainWindow : public QMainWindow
{
public:
  MainWindow(vision::gui::Controller& controller)
    : m_central_widget(vision::gui::CreatePage(this, controller))
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

  std::unique_ptr<vision::gui::Controller> controller =
    vision::gui::Controller::Create();

  MainWindow main_window(*controller);

  main_window.show();

  return app.exec();
}
