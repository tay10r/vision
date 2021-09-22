#include "schedule.hpp"

#include "vertex.hpp"

#include <set>

#include <stdint.h>

namespace vision::gui {

namespace {

inline uint32_t
ReverseInterleave(uint64_t n)
{
  n &= 0x5555555555555555ull;
  n = (n ^ (n >> 1)) & 0x3333333333333333ull;
  n = (n ^ (n >> 2)) & 0x0f0f0f0f0f0f0f0full;
  n = (n ^ (n >> 4)) & 0x00ff00ff00ff00ffull;
  n = (n ^ (n >> 8)) & 0x0000ffff0000ffffull;
  n = (n ^ (n >> 16)) & 0x00000000ffffffffull;
  return (uint32_t)n;
}

inline uint32_t
ReverseInterleaveX(uint64_t n)
{
  return ReverseInterleave(n);
}

inline uint32_t
ReverseInterleaveY(uint64_t n)
{
  return ReverseInterleave(n >> 1);
}

inline size_t
Log4(size_t n)
{
  size_t out = 0;

  while (n > 3) {
    n >>= 2;
    out++;
  }

  return out;
}

} // namespace

Schedule::Schedule(size_t w, size_t h, size_t division_level)
  : m_width(w)
  , m_height(h)
  , m_division_level(division_level)
{
  const size_t x_pixel_count = GetTextureWidth() / GetDivisionsPerDimension();
  const size_t y_pixel_count = GetTextureHeight() / GetDivisionsPerDimension();

  const size_t x_pixel_stride = GetHorizontalStride();
  const size_t y_pixel_stride = GetVerticalStride();

  std::set<size_t> indices;

  const size_t div_count = GetDivisionsPerDimension();

  for (size_t i = 0; i < m_division_level; i++) {

    const size_t j_max = (1 << i) * (1 << i);

    const size_t stride = (div_count * div_count) / j_max;

    for (size_t j = 0; j < j_max; j++) {

      const size_t index = j * stride;

      const bool exists = (indices.find(index) != indices.end());
      if (exists)
        continue;

      const size_t x_cnt = (div_count > 1) ? x_pixel_count - 1 : x_pixel_count;
      const size_t y_cnt = (div_count > 1) ? y_pixel_count - 1 : y_pixel_count;

      const RenderRequest req{ x_cnt,
                               y_cnt,
                               ReverseInterleaveX(index),
                               ReverseInterleaveY(index),
                               x_pixel_stride,
                               y_pixel_stride,
                               m_width,
                               m_height };

      m_render_requests.emplace_back(req);

      indices.emplace(index);
    }
  }
}

size_t
Schedule::GetRenderRequestCount() const noexcept
{
  return m_render_requests.size();
}

size_t
Schedule::GetRemainingRenderRequests() const noexcept
{
  if (m_render_request_index > m_render_requests.size())
    return 0;
  else
    return m_render_requests.size() - m_render_request_index;
}

size_t
Schedule::GetPartitionCount() const noexcept
{
  const size_t divs = GetDivisionsPerDimension();

  return divs * divs;
}

size_t
Schedule::GetPartitionWidth() const noexcept
{
  return GetTextureWidth() / GetDivisionsPerDimension();
}

size_t
Schedule::GetPartitionHeight() const noexcept
{
  return GetTextureHeight() / GetDivisionsPerDimension();
}

bool
Schedule::HasPreview() const noexcept
{
  return m_render_request_index > 0;
}

size_t
Schedule::GetPreviewIndex() const noexcept
{
  return Log4(m_render_request_index);
}

std::vector<PreviewOperation>
Schedule::GetPreviewOperations() const
{
  std::vector<PreviewOperation> operations;

  if (!HasPreview())
    return operations;

  const size_t div_count = 1 << GetPreviewIndex();

  std::set<size_t> indices;

  for (size_t i = 0; i < div_count; i++) {

    const size_t j_max = (1 << i) * (1 << i);

    const size_t stride = (div_count * div_count) / j_max;

    for (size_t j = 0; j < j_max; j++) {

      const size_t index = j * stride;

      const bool exists = (indices.find(index) != indices.end());
      if (exists)
        continue;

      const size_t x_pixel_offset = ReverseInterleaveX(index);
      const size_t y_pixel_offset = ReverseInterleaveY(index);

      const PreviewOperation op{
        x_pixel_offset, y_pixel_offset, div_count, div_count
      };

      operations.emplace_back(op);

      indices.emplace(index);
    }
  }

  return operations;
}

size_t
Schedule::GetPreviewCount() const noexcept
{
  return m_division_level;
}

size_t
Schedule::GetTextureWidth() const noexcept
{
  const size_t divs = GetDivisionsPerDimension();

  return ((m_width + (divs - 1)) / divs) * divs;
}

size_t
Schedule::GetTextureHeight() const noexcept
{
  const size_t divs = GetDivisionsPerDimension();

  return ((m_height + (divs - 1)) / divs) * divs;
}

size_t
Schedule::GetVerticalStride() const noexcept
{
  return GetDivisionsPerDimension();
}

size_t
Schedule::GetHorizontalStride() const noexcept
{
  // return GetTextureWidth() / GetDivisionsPerDimension();
  return GetDivisionsPerDimension();
}

size_t
Schedule::GetDivisionsPerDimension() const noexcept
{
  return 1 << m_division_level;
}

std::vector<Vertex>
Schedule::GetVertexBuffer() const
{
  const size_t partition_w = GetPartitionWidth();
  const size_t partition_h = GetPartitionHeight();

  const float x_scale = 1.0f / partition_w;
  const float y_scale = 1.0f / partition_h;

  std::vector<Vertex> vertices(partition_w * partition_h * 6);

  for (size_t y = 0; y < partition_h; y++) {

    for (size_t x = 0; x < partition_w; x++) {

      const float x0 = (x + 0) * x_scale;
      const float y0 = (y + 0) * y_scale;

      const float x1 = (x + 1) * x_scale;
      const float y1 = (y + 1) * y_scale;

      const float dx0 = 0.0f;
      const float dy0 = 0.0f;

      const float dx1 = x1 - x0;
      const float dy1 = y1 - y0;

      const Vertex p00(x0, y0, dx0, dy0);
      const Vertex p10(x0, y0, dx1, dy0);
      const Vertex p01(x0, y0, dx0, dy1);
      const Vertex p11(x0, y0, dx1, dy1);

      size_t i = ((y * partition_w) + x) * 6;

      vertices[i + 0] = p00;
      vertices[i + 1] = p01;
      vertices[i + 2] = p10;

      vertices[i + 3] = p01;
      vertices[i + 4] = p11;
      vertices[i + 5] = p10;
    }
  }

  return vertices;
}

RenderRequest
Schedule::GetRenderRequest() const
{
  if (m_render_request_index >= m_render_requests.size())
    return RenderRequest();
  else
    return m_render_requests[m_render_request_index];
}

void
Schedule::NextRenderRequest()
{
  m_render_request_index++;
}

} // namespace vision::gui
