#pragma once

#include "connection.hpp"

#include <memory>

namespace vision::gui {

auto
CreateDebugInputConnection(ConnectionObserver&) -> std::unique_ptr<Connection>;

} // namespace vision::gui
