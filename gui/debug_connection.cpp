#include "debug_connection.hpp"

#include "debug_render.hpp"

#include <QString>

namespace vision::gui {

auto
CreateDebugConnection(ConnectionObserver& observer, const QString& address_data)
  -> std::unique_ptr<Connection>
{
  if (address_data == "render")
    return CreateDebugRenderer(observer);

  return nullptr;
}

} // namespace vision::gui
