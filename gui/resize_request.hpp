#pragma once

namespace vision::gui {

struct ResizeRequest final
{
  size_t width = 1;

  size_t height = 1;

  size_t padded_width = 1;

  size_t padded_height = 1;
};

} // namespace vision::gui
