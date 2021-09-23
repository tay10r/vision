#pragma once

#include "render_request.hpp"

#include <vector>

namespace vision::gui {

struct Vertex;

class IDGenerator;

/// Describes how to render the results of a render request for a given preview
/// index.
struct PreviewOperation final
{
  size_t x_pixel_offset = 0;
  size_t y_pixel_offset = 0;

  size_t x_pixel_stride = 0;
  size_t y_pixel_stride = 0;
};

/// Used for scheduling the rendering of a frame. Divides the frame into
/// partitions, each with a different starting offset and stride. While each
/// partition is received, there are several points in which a preview of the
/// frame is available. The number of partitions depends on the division level.
class Schedule final
{
public:
  Schedule(size_t w,
           size_t h,
           size_t division_level,
           IDGenerator& id_generator);

  size_t GetRemainingRenderRequests() const noexcept;

  size_t GetRenderRequestCount() const noexcept;

  size_t GetPartitionCount() const noexcept;

  bool HasPreview() const noexcept;

  size_t GetPreviewIndex() const noexcept;

  size_t GetPreviewCount() const noexcept;

  size_t GetPartitionWidth() const noexcept;

  size_t GetPartitionHeight() const noexcept;

  size_t GetTextureWidth() const noexcept;

  size_t GetTextureHeight() const noexcept;

  size_t GetFrameWidth() const noexcept { return m_width; }

  size_t GetFrameHeight() const noexcept { return m_height; }

  size_t GetVerticalStride() const noexcept;

  size_t GetHorizontalStride() const noexcept;

  std::vector<PreviewOperation> GetPreviewOperations() const;

  std::vector<Vertex> GetVertexBuffer() const;

  RenderRequest GetRenderRequest(size_t index) const;

  RenderRequest GetRenderRequest() const;

  void NextRenderRequest();

private:
  size_t GetDivisionsPerDimension() const noexcept;

private:
  std::vector<RenderRequest> m_render_requests;

  size_t m_render_request_index = 0;

  size_t m_width = 0;

  size_t m_height = 0;

  size_t m_division_level = 0;
};

} // namespace vision::gui
