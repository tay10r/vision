#pragma once

#include <memory>

class QString;

namespace vision::gui {

class Connection;
class ConnectionObserver;

auto
CreateDebugConnection(ConnectionObserver& observer, const QString& address_data)
  -> std::unique_ptr<Connection>;

} // namespace vision::gui
