#pragma once

class QWidget;

namespace vision::gui {

class Controller;

QWidget*
CreateTabWidget(QWidget* parent, Controller&);

} // namespace vision::gui
