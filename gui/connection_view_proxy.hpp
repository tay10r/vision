#pragma once

#include <memory>

namespace vision::gui {

class Connection;
class ViewObserver;

auto
CreateConnectionViewProxy(Connection& connection)
  -> std::unique_ptr<ViewObserver>;

} // namespace vision::gui
