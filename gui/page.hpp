#pragma once

class QWidget;

namespace vision::gui {

class Controller;

QWidget*
CreatePage(QWidget* parent, Controller& controller);

} // namespace vision::gui
