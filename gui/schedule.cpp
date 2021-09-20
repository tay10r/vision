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
Log2(size_t n)
{
  if (n == 0)
    return 1;

  size_t out = 0;

  while (n > 1) {
    n >>= 1;
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
  const size_t count = m_division_level * m_division_level;

  const size_t x_pixel_count = GetTextureWidth() / GetDivisionsPerDimension();
  const size_t y_pixel_count = GetTextureHeight() / GetDivisionsPerDimension();

  const size_t x_pixel_stride = GetHorizontalStride();
  const size_t y_pixel_stride = GetVerticalStride();

  std::set<size_t> indices;

  const size_t div_count = GetDivisionsPerDimension();

  const size_t j_max = div_count * div_count;

  for (size_t i = 0; i < div_count; i++) {

    const size_t j_stride = j_max / ((i + 1) * (i + 1));

    for (size_t j = 0; j < j_max; j += j_stride) {

      const bool exists = (indices.find(j) != indices.end());

      if (exists)
        continue;

      const size_t x_cnt = (div_count > 1) ? x_pixel_count - 1 : x_pixel_count;
      const size_t y_cnt = (div_count > 1) ? y_pixel_count - 1 : y_pixel_count;

      const RenderRequest req{ 0 /* unique ID */,
                               x_cnt,
                               y_cnt,
                               ReverseInterleaveX(j),
                               ReverseInterleaveY(j),
                               x_pixel_stride,
                               y_pixel_stride,
                               m_width,
                               m_height };

      m_render_requests.emplace_back(req);

      indices.emplace(j);
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

bool
Schedule::HasPreview() const noexcept
{
  return m_render_request_index > 0;
}

std::vector<PreviewOperation>
Schedule::GetPreviewOperations() const
{
  std::vector<PreviewOperation> operations;

  if (!HasPreview())
    return operations;

  const size_t div_count = GetPreviewIndex();

  const size_t j_max = div_count * div_count;

  std::set<size_t> indices;

  for (size_t i = 0; i < div_count; i++) {

    const size_t j_stride = j_max / ((i + 1) * (i + 1));

    for (size_t j = 0; j < j_max; j += j_stride) {

      const bool exists = (indices.find(j) != indices.end());
      if (exists)
        continue;

      const size_t x_pixel_offset = ReverseInterleaveX(j);
      const size_t y_pixel_offset = ReverseInterleaveY(j);

      const PreviewOperation op{
        x_pixel_offset, y_pixel_offset, div_count, div_count
      };

      operations.emplace_back(op);

      indices.emplace(j);
    }
  }

  return operations;
}

size_t
Schedule::GetPreviewIndex() const noexcept
{
  return Log2(m_render_request_index + 1);
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
  const size_t tex_w = GetTextureWidth();
  const size_t tex_h = GetTextureHeight();

  const float x_scale = 1.0f / tex_w;
  const float y_scale = 1.0f / tex_h;

  std::vector<Vertex> vertices(tex_w * tex_h * 6);

  for (size_t y = 0; y < tex_h; y++) {

    for (size_t x = 0; x < tex_w; x++) {

      const Vertex p00((x + 0) * x_scale, (y + 0) * y_scale);
      const Vertex p10((x + 1) * x_scale, (y + 0) * y_scale);
      const Vertex p01((x + 0) * x_scale, (y + 1) * y_scale);
      const Vertex p11((x + 1) * x_scale, (y + 1) * y_scale);

      size_t i = ((y * tex_w) + x) * 6;

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
