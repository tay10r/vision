#pragma once

class QWidget;
class QString;

namespace vision::gui {

class AddressBarObserver
{
public:
  virtual ~AddressBarObserver() = default;

  virtual void OnConnectionRequest(const QString& url) = 0;

  virtual void OnMonitorVisibilityToggle(bool visible) = 0;
};

QWidget*
CreateAddressBar(QWidget* parent, AddressBarObserver& observer);

} // namespace vision::gui
