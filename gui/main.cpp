#include <QApplication>
#include <QMainWindow>
#include <QSurfaceFormat>

#include "page.hpp"

namespace {

class MainWindow : public QMainWindow
{
public:
  MainWindow()
    : m_page(vision::gui::CreatePage(this))
  {
    resize(1280, 720);

    setWindowTitle("Vision");

    setCentralWidget(m_page);
  }

protected:
  void closeEvent(QCloseEvent* closeEvent) override
  {
    m_page->PrepareToClose();

    QMainWindow::closeEvent(closeEvent);
  }

private:
  vision::gui::Page* m_page;
};

const char* stylesheet = R"(
QWidget {
  color: white;
  background-color: #181818
}

QLineEdit {
  background-color: #222222;
  border-radius: 4px;
  padding: 5px
}

QTextEdit {
  background-color: #222222
}

QPushButton {
  color: orange;
  border: 0px solid black;
  border-radius: 8px;
  padding: 5px;
}

QPushButton::menu-indicator {
  image:none;
  width: 0
}

QPushButton:hover {
  background-color: #333333
}

QComboBox {
  color: #c576f6;
  border: 0px solid black;
  border-radius: 8px;
  padding: 5px
}

QComboBox QAbstractItemView {
  border: 1px solid #444444;
  border-radius: 8px;
  padding: 5px
}

QComboBox::drop-down {
  image: none
}

QComboBox::down-arrow {
  image: none
}

QFrame#AddressBar {
  border: 1px none #444444;
  border-bottom-style: solid;
  padding: 0
}

QMenu {
  color: orange;
  border: 0px solid #444444;
  border-radius: 8px
}
)";

} // namespace

int
main(int argc, char** argv)
{
  QApplication app(argc, argv);

  app.setStyleSheet(stylesheet);

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
