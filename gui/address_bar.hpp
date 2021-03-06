#pragma once

#include <QComboBox>
#include <QCompleter>
#include <QFileSystemModel>
#include <QFrame>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QString>
#include <QStringListModel>

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

class AddressBar : public QFrame
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

  void OpenStyleSheetEditor();

private:
  QHBoxLayout m_layout{ this };

  QPushButton m_back_button{ "\u2190", this };

  QPushButton m_forward_button{ "\u2192", this };

  QPushButton m_refresh_button{ "\u27f3", this };

  QComboBox m_address_kind_box{ this };

  QLineEdit m_line_edit{ this };

  QPushButton m_menu_button{ "\u2630", this };

  QCompleter m_completer{ this };

  QStringListModel m_debug_item_model{ this };

  QFileSystemModel m_fs_model{ this };

  QStringListModel m_tcp_item_model{ this };

  QMenu m_menu{ this };

  std::vector<Address> m_history;
};

} // namespace vision::gui
