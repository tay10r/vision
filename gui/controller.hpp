#pragma once

#include <memory>

class QString;
class QAbstractItemModel;

namespace vision::gui {

class AddressBarFactory;
class Connection;
class ConnectionObserver;

class Controller
{
public:
  static auto Create() -> std::unique_ptr<Controller>;

  virtual ~Controller() = default;

  virtual auto CreateConnection(const QString&, ConnectionObserver& observer)
    -> std::unique_ptr<Connection> = 0;

  virtual auto CreateAddressBarFactory()
    -> std::unique_ptr<AddressBarFactory> = 0;
};

} // namespace vision::gui
