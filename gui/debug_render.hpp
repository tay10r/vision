#pragma once

#include "connection.hpp"

#include <memory>

namespace vision::gui {

auto
CreateDebugRenderer(ConnectionObserver&) -> std::unique_ptr<Connection>;

} // namespace vision::gui
