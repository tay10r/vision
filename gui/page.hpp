#pragma once

#include <QWidget>

class QString;

namespace vision::gui {

class Page : public QWidget
{
  Q_OBJECT
public:
  Page(QWidget* parent)
    : QWidget(parent)
  {}

  virtual void PrepareToClose() = 0;
};

Page*
CreatePage(QWidget* parent);

} // namespace vision::gui
