#include "central_widget.hpp"

#include "controller.hpp"
#include "view_page.hpp"

#include <QLabel>
#include <QTabBar>
#include <QTabWidget>

namespace vision::gui {

namespace {

class AddTabWidget : public QLabel
{
public:
  AddTabWidget(QWidget* parent)
    : QLabel(parent)
  {
    setText(tr("Create a new tab by clicking the \"+\" button."));

    setAlignment(Qt::AlignCenter);
  }
};

class CentralWidget : public QTabWidget
{
public:
  CentralWidget(QWidget* parent, Controller& controller)
    : QTabWidget(parent)
    , m_controller(controller)
  {
    addTab(&m_add_tab_widget, "+");

    AddViewPage();

    SortAddButton();

    setCurrentIndex(0);

    connect(
      this, &QTabWidget::tabBarClicked, this, &CentralWidget::OnTabClicked);
  }

protected slots:
  void OnTabClicked(int tabIndex)
  {
    if (tabIndex == indexOf(&m_add_tab_widget))
      AddViewPage();
  }

private:
  void SortAddButton()
  {
    const int addTabIndex = indexOf(&m_add_tab_widget);

    removeTab(addTabIndex);

    addTab(&m_add_tab_widget, "+");
  }

  int AddViewPage()
  {
    QWidget* view_page = CreateViewPage(this, m_controller);

    addTab(view_page, "New Tab");

    SortAddButton();

    return indexOf(view_page);
  }

private:
  AddTabWidget m_add_tab_widget{ this };

  Controller& m_controller;
};

} // namespace

QWidget*
CreateCentralWidget(QWidget* parent, Controller& controller)
{
  return new CentralWidget(parent, controller);
}

} // namespace vision::gui
