#include "address_bar.hpp"

namespace vision::gui {

AddressBar::AddressBar(QWidget* parent)
  : QWidget(parent)
{
  m_layout.addWidget(&m_back_button);

  m_layout.addWidget(&m_forward_button);

  m_layout.addWidget(&m_refresh_button);

  m_layout.addWidget(&m_address_kind_box);

  m_layout.addWidget(&m_line_edit);

  m_layout.addWidget(&m_menu_button);

  m_address_kind_box.addItem("\U0001f310", QString("tcp"));
  m_address_kind_box.addItem("\U0001f4c1", QString("file"));
  m_address_kind_box.addItem("\U0001f41b", QString("debug"));

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

} // namespace vision::gui
