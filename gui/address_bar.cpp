#include "address_bar.hpp"

#include <QApplication>
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>

#include <fstream>

namespace vision::gui {

namespace {

class StyleSheetEditor final : public QDialog
{
public:
  StyleSheetEditor()
  {
    setWindowTitle("Style Sheet Editor");

    resize(640, 720);

    setLayout(&m_layout);

    m_layout.addWidget(&m_text_edit);

    m_layout.addWidget(&m_apply_button);

    m_text_edit.insertPlainText(qApp->styleSheet());

    connect(&m_apply_button,
            &QPushButton::clicked,
            this,
            &StyleSheetEditor::ApplyStyleSheet);
  }

protected slots:
  void ApplyStyleSheet()
  {
    const QString style_sheet = m_text_edit.toPlainText();

    qApp->setStyleSheet(style_sheet);

    std::ofstream file("style.css");

    file << style_sheet.toStdString();
  }

private:
  QVBoxLayout m_layout{ this };

  QTextEdit m_text_edit{ this };

  QPushButton m_apply_button{ tr("Apply"), this };
};

} // namespace

AddressBar::AddressBar(QWidget* parent)
  : QFrame(parent)
{
  setObjectName("AddressBar");

  m_layout.addWidget(&m_back_button);
  m_layout.addWidget(&m_forward_button);
  m_layout.addWidget(&m_refresh_button);
  m_layout.addWidget(&m_address_kind_box);
  m_layout.addWidget(&m_line_edit);
  m_layout.addWidget(&m_menu_button);

  m_address_kind_box.addItem("tcp", QString("tcp"));
  m_address_kind_box.addItem("file", QString("file"));
  m_address_kind_box.addItem("debug", QString("debug"));

  m_menu_button.setMenu(&m_menu);

  QAction* edit_style_sheet_action = m_menu.addAction("Edit Style Sheet");

  connect(edit_style_sheet_action,
          &QAction::triggered,
          this,
          &AddressBar::OpenStyleSheetEditor);

  QStringList debug_item_list;
  debug_item_list << "render";
  debug_item_list << "buffer_overflow";
  debug_item_list << "bad_connection";
  debug_item_list << "invalid_response";
  m_debug_item_model.setStringList(debug_item_list);

  m_fs_model.setRootPath("/");

  m_line_edit.setCompleter(&m_completer);

  ToTcpMode();

  connect(&m_address_kind_box,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &AddressBar::HandleModeSwitch);

  connect(&m_refresh_button,
          &QPushButton::clicked,
          this,
          &AddressBar::EmitConnectRequest);

  connect(&m_line_edit,
          &QLineEdit::returnPressed,
          this,
          &AddressBar::EmitConnectRequest);
}

void
AddressBar::HandleModeSwitch(int index)
{
  switch (GetAddressKind(m_address_kind_box.itemData(index).toString())) {
    case AddressKind::Unknown:
      break;
    case AddressKind::Debug:
      ToDebugMode();
      break;
    case AddressKind::File:
      ToFileMode();
      break;
    case AddressKind::Tcp:
      ToTcpMode();
      break;
  }
}

void
AddressBar::EmitConnectRequest()
{
  Address addr{ GetCurrentAddressKind(), m_line_edit.text() };

  emit ConnectionRequest(addr);
}

void
AddressBar::SwitchMode(QAbstractItemModel* model,
                       const QString& placeholder_text)
{
  m_completer.setModel(model);

  m_line_edit.setPlaceholderText(placeholder_text);

  m_line_edit.clear();
}

AddressKind
AddressBar::GetCurrentAddressKind() const
{
  return GetAddressKind(m_address_kind_box.currentData().toString());
}

AddressKind
AddressBar::GetAddressKind(const QString& kind)
{
  if (kind == "tcp")
    return AddressKind::Tcp;
  else if (kind == "file")
    return AddressKind::File;
  else if (kind == "debug")
    return AddressKind::Debug;

  return AddressKind::Unknown;
}

void
AddressBar::ToTcpMode()
{
  SwitchMode(&m_tcp_item_model, "Enter an address to connect to.");
}

void
AddressBar::ToFileMode()
{
  SwitchMode(&m_fs_model, "Enter a program to launch.");
}

void
AddressBar::ToDebugMode()
{
  SwitchMode(&m_debug_item_model, "Enter an aspect to debug.");
}

void
AddressBar::OpenStyleSheetEditor()
{
  StyleSheetEditor editor;

  editor.exec();
}

} // namespace vision::gui
