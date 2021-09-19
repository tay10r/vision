#pragma once

class QWidget;

namespace vision::gui {

class Controller;

QWidget*
CreateViewPage(QWidget* parent, Controller& controller);

} // namespace vision::gui
