#pragma once

#include <memory>

namespace vision::gui {

class Controller;

std::unique_ptr<Controller>
CreateFakeController();

} // namespace vision::gui
