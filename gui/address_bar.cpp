#include "address_bar.hpp"

#include <QFocusEvent>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

namespace vision::gui {

namespace {

class Menu final : public QMenu
{
public:
  Menu(QPushButton* button, QWidget* parent)
    : QMenu(parent)
    , m_button(button)
  {
    QAction* view_log_action = addAction("View Log");

    view_log_action->setCheckable(true);

    addAction("About");
  }

protected:
  void showEvent(QShowEvent*) override
  {
    const QPoint p = this->pos();

    const QRect geo = m_button->geometry();

    this->move(p.x() + geo.width() - this->geometry().width(), p.y());
  }

private:
  QPushButton* m_button;
};

class AddressBarButton final : public QPushButton
{
public:
  AddressBarButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
  {
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  }
};

class AddressEdit : public QLineEdit
{
public:
  AddressEdit(QWidget* parent)
    : QLineEdit(parent)
  {
    setPlaceholderText("Enter an address to connect to.");
  }

protected:
  void focusInEvent(QFocusEvent* event) override
  {
    if (event->reason() == Qt::MouseFocusReason)
      QTimer::singleShot(0, this, &QLineEdit::selectAll);

    QLineEdit::focusInEvent(event);
  }
};

class AddressBar final : public QWidget
{
public:
  AddressBar(QWidget* parent, AddressBarObserver& observer)
    : QWidget(parent)
    , m_observer(observer)
  {
    m_layout.addWidget(&m_line_edit);

    m_layout.addWidget(&m_menu_button);

    setSizePolicy(sizePolicy().horizontalPolicy(), QSizePolicy::Minimum);

    connect(&m_line_edit,
            &QLineEdit::returnPressed,
            this,
            &AddressBar::OnReturnPressed);

    m_menu_button.setMenu(&m_menu);
    // connect(&m_menu_button, &QPushButton::clicked, this,
    // &AddressBar::ShowMenu);
  }

protected slots:
  void OnReturnPressed() { m_observer.OnConnectionRequest(m_line_edit.text()); }

  // void ShowMenu() { m_menu.show(); }

private:
  AddressBarObserver& m_observer;

  AddressBarButton m_menu_button{ "\u2630", this };

  QHBoxLayout m_layout{ this };

  AddressEdit m_line_edit{ this };

  Menu m_menu{ &m_menu_button, this };
};

} // namespace

QWidget*
CreateAddressBar(QWidget* parent, AddressBarObserver& observer)
{
  return new AddressBar(parent, observer);
}

} // namespace vision::gui
