#pragma once

#include <QComboBox>
#include <QCompleter>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QStringListModel>
#include <QWidget>

#include <vector>

namespace vision::gui {

enum class AddressKind
{
  Unknown,
  Debug,
  File,
  Tcp
};

struct Address final
{
  AddressKind kind;

  QString data;
};

class AddressBar : public QWidget
{
  Q_OBJECT
public:
  AddressBar(QWidget* parent);

private:
  void SwitchMode(QAbstractItemModel* model, const QString& placeholder_text);

  void ToTcpMode();

  void ToFileMode();

  void ToDebugMode();

signals:
  void ConnectionRequest(const Address&);

protected slots:
  void EmitConnectRequest();

  void HandleModeSwitch(int index);

protected:
  AddressKind GetCurrentAddressKind() const;

  static AddressKind GetAddressKind(const QString&);

private:
  QHBoxLayout m_layout{ this };

  QPushButton m_back_button{ "\u2190", this };

  QPushButton m_forward_button{ "\u2192", this };

  QPushButton m_refresh_button{ "\u27f3", this };

  QComboBox m_address_kind_box{ this };

  QLineEdit m_line_edit{ this };

  QPushButton m_menu_button{ "\u2630", this };

  QCompleter m_completer{ this };

  QStringListModel m_debug_item_model;

  QFileSystemModel m_fs_model;

  QStringListModel m_tcp_item_model;

  std::vector<Address> m_history;
};

} // namespace vision::gui
