#include <QApplication>
#include <QMainWindow>
#include <QSurfaceFormat>

#include "central_widget.hpp"
#include "controller.hpp"

namespace {

class MainWindow : public QMainWindow
{
public:
  MainWindow(vision::gui::Controller& controller)
    : m_central_widget(vision::gui::CreateCentralWidget(this, controller))
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
