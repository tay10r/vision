#pragma once

#include <stddef.h>

namespace vision::gui {

/// Describes which pixels should be rendered next. These results of these
/// requests are agregated to form a full frame.
struct RenderRequest final
{
  size_t id = 0;

  size_t x_pixel_count = 0;
  size_t y_pixel_count = 0;

  size_t x_pixel_offset = 0;
  size_t y_pixel_offset = 0;

  size_t x_pixel_stride = 0;
  size_t y_pixel_stride = 0;

  size_t x_frame_size = 0;
  size_t y_frame_size = 0;

  constexpr size_t GetFrameX(size_t in_x) const noexcept
  {
    return x_pixel_offset + (in_x * x_pixel_stride);
  }

  constexpr size_t GetFrameY(size_t in_y) const noexcept
  {
    return y_pixel_offset + (in_y * y_pixel_stride);
  }

  constexpr bool IsValid() const noexcept
  {
    return (x_frame_size > 0) && (y_frame_size > 0);
  }
};

} // namespace vision::gui
