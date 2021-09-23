#include "address_bar.hpp"

#include "auto_complete_engine.hpp"

#include <QComboBox>
#include <QCompleter>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QStringListModel>
#include <QTimer>
#include <QWidget>

#include <map>
#include <vector>

#include <QDebug>

namespace vision::gui {

namespace {

class Menu final : public QMenu
{
public:
  Menu(AddressBarObserver& observer, QPushButton* button, QWidget* parent)
    : QMenu(parent)
    , m_observer(observer)
    , m_button(button)
  {
    QAction* view_monitor_action = addAction("View Monitor");

    view_monitor_action->setCheckable(true);

    connect(
      view_monitor_action, &QAction::toggled, this, &Menu::OnMonitorToggle);

    addAction("About");
  }

protected slots:
  void OnMonitorToggle(bool checked)
  {
    m_observer.OnMonitorVisibilityToggle(checked);
  }

protected:
  void showEvent(QShowEvent*) override
  {
    const QPoint p = this->pos();

    const QRect geo = m_button->geometry();

    this->move(p.x() + geo.width() - this->geometry().width(), p.y());
  }

private:
  AddressBarObserver& m_observer;

  QPushButton* m_button;
};

class AddressBarButton final : public QPushButton
{
public:
  AddressBarButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
  {
    setFlat(true);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  }
};

class AddressEdit : public QLineEdit
{
public:
  AddressEdit(std::unique_ptr<AutoCompleteEngine>&& ac_engine, QWidget* parent)
    : QLineEdit(parent)
    , m_auto_complete_engine(std::move(ac_engine))
  {
    m_auto_complete_engine->setParent(this);

    setPlaceholderText("Enter an address to connect to.");

    m_completer.setModel(&m_completion_model);

    setCompleter(&m_completer);

    connect(this, &QLineEdit::textEdited, this, &AddressEdit::UpdateCompleter);
  }

protected slots:
  void UpdateCompleter(const QString& url_string)
  {
    m_auto_complete_engine->Execute(url_string);

    qDebug() << m_auto_complete_engine->GetResults();

    m_completion_model.setStringList(m_auto_complete_engine->GetResults());
  }

protected:
  void focusInEvent(QFocusEvent* event) override
  {
    if (event->reason() == Qt::MouseFocusReason)
      QTimer::singleShot(0, this, &QLineEdit::selectAll);

    QLineEdit::focusInEvent(event);
  }

private:
  std::unique_ptr<AutoCompleteEngine> m_auto_complete_engine;

  QCompleter m_completer;

  QStringListModel m_completion_model;
};

class AddressBar final : public QWidget
{
public:
  AddressBar(QWidget* parent,
             std::unique_ptr<AutoCompleteEngine>&& ac_engine,
             std::unique_ptr<AddressBarObserver>&& observer)
    : QWidget(parent)
    , m_observer(std::move(observer))
    , m_line_edit(std::move(ac_engine), this)
  {
    m_address_kind_box.addItem(QString("\U0001f310"));
    m_address_kind_box.addItem(QString("\U0001f4c1"));

    m_layout.addWidget(&m_address_kind_box);

    m_layout.addWidget(&m_line_edit);

    m_layout.addWidget(&m_menu_button);

    setSizePolicy(sizePolicy().horizontalPolicy(), QSizePolicy::Minimum);

    connect(&m_line_edit,
            &QLineEdit::returnPressed,
            this,
            &AddressBar::OnReturnPressed);

    m_menu_button.setMenu(&m_menu);
  }

protected slots:
  void OnReturnPressed()
  {
    m_observer->OnConnectionRequest(m_line_edit.text());
  }

private:
  std::unique_ptr<AddressBarObserver> m_observer;

  QComboBox m_address_kind_box{ this };

  AddressBarButton m_menu_button{ "\u2630", this };

  QHBoxLayout m_layout{ this };

  AddressEdit m_line_edit;

  Menu m_menu{ *m_observer, &m_menu_button, this };
};

} // namespace

namespace {

class CompositeObserver final : public AddressBarObserver
{
public:
  void AddObserver(AddressBarObserver* observer)
  {
    m_observers.emplace_back(observer);
  }

  void OnConnectionRequest(const QString& url) override
  {
    for (auto* observer : m_observers)
      observer->OnConnectionRequest(url);
  }

  void OnMonitorVisibilityToggle(bool visible) override
  {
    for (auto* observer : m_observers)
      observer->OnMonitorVisibilityToggle(visible);
  }

private:
  std::vector<AddressBarObserver*> m_observers;
};

class AddressBarFactoryImpl final : public AddressBarFactory
{
public:
  void AddScheme(const QString& scheme, QAbstractItemModel* model) override
  {
    if (!m_auto_complete_engine)
      m_auto_complete_engine.reset(new AutoCompleteEngine(nullptr));

    m_auto_complete_engine->AddModel(scheme, model);
  }

  void AddObserver(AddressBarObserver* observer) override
  {
    if (!m_observer)
      m_observer.reset(new CompositeObserver());

    m_observer->AddObserver(observer);
  }

  auto CreateAddressBar(QWidget* parent) -> QWidget* override
  {
    if (!m_observer)
      m_observer.reset(new CompositeObserver());

    if (!m_auto_complete_engine)
      m_auto_complete_engine.reset(new AutoCompleteEngine(nullptr));

    return new AddressBar(
      parent, std::move(m_auto_complete_engine), std::move(m_observer));
  }

private:
  std::unique_ptr<AutoCompleteEngine> m_auto_complete_engine{ nullptr };

  std::unique_ptr<CompositeObserver> m_observer;
};

} // namespace

auto
AddressBarFactory::Create() -> std::unique_ptr<AddressBarFactory>
{
  return std::unique_ptr<AddressBarFactory>(new AddressBarFactoryImpl());
}

} // namespace vision::gui
