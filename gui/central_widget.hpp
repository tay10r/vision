#pragma once

class QWidget;

namespace vision::gui {

class Controller;

QWidget*
CreateCentralWidget(QWidget* parent, Controller&);

} // namespace vision::gui
