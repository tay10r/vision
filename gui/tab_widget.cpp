#include "tab_widget.hpp"

#include "page.hpp"

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

class TabWidget : public QTabWidget
{
public:
  TabWidget(QWidget* parent, Controller& controller)
    : QTabWidget(parent)
    , m_controller(controller)
  {
    addTab(&m_add_tab_widget, "+");

    AddPage();

    SortAddButton();

    setCurrentIndex(0);

    setMovable(true);

    connect(this, &QTabWidget::tabBarClicked, this, &TabWidget::OnTabClicked);
  }

protected slots:
  void OnTabClicked(int tabIndex)
  {
    if (tabIndex == indexOf(&m_add_tab_widget))
      AddPage();
  }

private:
  void SortAddButton()
  {
    const int addTabIndex = indexOf(&m_add_tab_widget);

    removeTab(addTabIndex);

    addTab(&m_add_tab_widget, "+");
  }

  int AddPage()
  {
    QWidget* view_page = CreatePage(this, m_controller);

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
CreateTabWidget(QWidget* parent, Controller& controller)
{
  return new TabWidget(parent, controller);
}

} // namespace vision::gui
