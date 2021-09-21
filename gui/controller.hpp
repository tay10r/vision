#pragma once

#include <memory>

class QString;

namespace vision::gui {

class Connection;
class ConnectionObserver;

class Controller
{
public:
  static auto Create() -> std::unique_ptr<Controller>;

  virtual ~Controller() = default;

  virtual auto CreateConnection(const QString&, ConnectionObserver& observer)
    -> std::unique_ptr<Connection> = 0;
};

} // namespace vision::gui
