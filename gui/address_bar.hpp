#pragma once

#include <memory>

class QAbstractItemModel;
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

class AddressBarFactory
{
public:
  static auto Create() -> std::unique_ptr<AddressBarFactory>;

  virtual ~AddressBarFactory() = default;

  virtual void AddScheme(const QString& scheme, QAbstractItemModel* model) = 0;

  virtual void AddObserver(AddressBarObserver* observer) = 0;

  virtual auto CreateAddressBar(QWidget* parent) -> QWidget* = 0;
};

} // namespace vision::gui
