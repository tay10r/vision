#include "view.hpp"

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <QDebug>

#include <map>

#include <assert.h>
#include <stdint.h>

#include <iostream>

namespace vision::gui {

namespace {

const char* vertSrc = R"(
#version 330 core

layout(location = 0) in vec4 position;

uniform float u_offset = 0.0;
uniform float v_offset = 0.0;

uniform float u_stride = 0.0;
uniform float v_stride = 0.0;

out vec2 tex_coords;

void
main()
{
  tex_coords = position.xy;

  vec2 offset = vec2(u_offset, v_offset);

  vec2 p0 = (position.zw * vec2(u_stride, v_stride)) + (position.xy - position.zw);

  vec2 p1 = p0 / vec2(u_stride, v_stride);

  vec2 p2 = ((p1 + offset) * 2.0) - 1.0;

  gl_Position = vec4(p2, 0.0, 1.0);
}
)";

const char* fragSrc = R"(
#version 330 core

out vec4 color;

uniform sampler2D partition;

varying vec2 tex_coords;

void
main()
{
  color = texture(partition, tex_coords);
}
)";

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

template<typename Scalar>
constexpr Scalar
Abs(Scalar n)
{
  return (n < 0) ? -n : n;
}

constexpr int
PadToPartitionLevel(int k, int partition_level)
{
  return ((k + (partition_level - 1)) / partition_level) * partition_level;
}

class IDGenerator final
{
public:
  IDGenerator(int init = 0)
    : m_next(init)
  {}

  auto Generate() -> int { return m_next++; }

private:
  int m_next;
};

struct RenderReply final
{
  std::vector<unsigned char> rgb_buffer;

  QOpenGLTexture texture{ QOpenGLTexture::Target2D };

  RenderReply(const RenderRequest* req, std::vector<unsigned char>&& data)
    : rgb_buffer(std::move(data))
    , texture(QImage(rgb_buffer.data(),
                     req->x_pixel_count,
                     req->y_pixel_count,
                     req->x_pixel_count * 3,
                     QImage::Format_RGB888),
              QOpenGLTexture::DontGenerateMipMaps)
  {}
};

class FrameBuildContext final
{
public:
  FrameBuildContext(const QSize& size, int partition_level, IDGenerator& id_gen)
    : FrameBuildContext(size.width(), size.height(), partition_level, id_gen)
  {}

  FrameBuildContext(int w, int h, int partition_level, IDGenerator& id_gen)
    : m_width(w)
    , m_height(h)
    , m_padded_width(PadToPartitionLevel(w, partition_level))
    , m_padded_height(PadToPartitionLevel(h, partition_level))
    , m_partition_level(partition_level)
    , m_partition_width(m_padded_width / partition_level)
    , m_partition_height(m_padded_height / partition_level)
    , m_vertex_buffer(QOpenGLBuffer::VertexBuffer)
  {
    InitVertexBuffer();

    InitRenderRequests(id_gen);
  }

  int GetVertexCount() const
  {
    const int tri_count =
      (m_partition_width - 1) * (m_partition_height - 1) * 2;

    const int verts_per_tri = 3;

    return tri_count * verts_per_tri;
  }

  QOpenGLBuffer& GetVertexBuffer() { return m_vertex_buffer; }

  const RenderRequest* PopRenderRequest()
  {
    if (m_render_request_index >= m_render_requests.size())
      return nullptr;

    const RenderRequest* req = &m_render_requests.at(m_render_request_index);

    m_render_request_index++;

    return req;
  }

  bool ReplyRenderRequest(const RenderRequest* req,
                          std::vector<unsigned char>&& data)
  {
    if (m_render_reply_index >= m_render_requests.size())
      return false;

    // This is the last render request that the user has taken.

    auto& current_req = m_render_requests[m_render_reply_index];

    // Verify the ID of the request given by the caller is the last render
    // request.

    if (current_req.unique_id == req->unique_id)
      return false;

    m_render_replies.emplace_back(new RenderReply(req, std::move(data)));

    const int next_partition_index = CheckAvailablePartitionIndex();

    if (next_partition_index > m_partition_index) {
      m_partition_index = next_partition_index;
      return true;
    } else {
      return false;
    }
  }

  bool HasParameters(int w, int h, int level) const
  {
    return (m_width == w) && (h == m_height) && (level == m_partition_level);
  }

  bool HasParameters(const QSize& size, int level) const
  {
    return HasParameters(size.width(), size.height(), level);
  }

  int CheckAvailablePartitionIndex() const
  {
    return Log2(m_render_replies.size());
  }

  int GetReplyCount() { return m_render_replies.size(); }

  const RenderRequest* GetRenderRequest(int index)
  {
    return &m_render_requests.at(index);
  }

  QOpenGLTexture* GetTexture(int index)
  {
    return &m_render_replies.at(index)->texture;
  }

private:
  static int Log2(unsigned int n)
  {
    unsigned int out = 0;

    while (n >>= 1u)
      out++;

    return out;
  }

  void InitVertexBuffer()
  {
    const int values_per_vertex = 4;

    const int x_max = m_partition_width - 1;

    const int y_max = m_partition_height - 1;

    const int quad_count = x_max * y_max;

    const int triangle_count = quad_count * 2;

    const int value_count = triangle_count * 3 * values_per_vertex;

    const int values_per_quad = values_per_vertex * 6;

    std::vector<float> vertices(value_count);

    const float pixel_w = 1.0f / m_partition_width;

    const float pixel_h = 1.0f / m_partition_height;

    for (int y = 0; y < y_max; y++) {

      for (int x = 0; x < x_max; x++) {

        int i = ((y * x_max) + x) * values_per_quad;

        const float p00[2]{ (x + 0) * pixel_w, (y + 0) * pixel_h };
        const float p01[2]{ (x + 0) * pixel_w, (y + 1) * pixel_h };
        const float p10[2]{ (x + 1) * pixel_w, (y + 0) * pixel_h };
        const float p11[2]{ (x + 1) * pixel_w, (y + 1) * pixel_h };

        // Triangle 1

        vertices[i + 0] = p00[0];
        vertices[i + 1] = p00[1];
        vertices[i + 2] = p00[0];
        vertices[i + 3] = p00[1];

        vertices[i + 4] = p01[0];
        vertices[i + 5] = p01[1];
        vertices[i + 6] = p00[0];
        vertices[i + 7] = p00[1];

        vertices[i + 8] = p10[0];
        vertices[i + 9] = p10[1];
        vertices[i + 10] = p00[0];
        vertices[i + 11] = p00[1];

        // Triangle 2

        vertices[i + 12] = p01[0];
        vertices[i + 13] = p01[1];
        vertices[i + 14] = p00[0];
        vertices[i + 15] = p00[1];

        vertices[i + 16] = p11[0];
        vertices[i + 17] = p11[1];
        vertices[i + 18] = p00[0];
        vertices[i + 19] = p00[1];

        vertices[i + 20] = p10[0];
        vertices[i + 21] = p10[1];
        vertices[i + 22] = p00[0];
        vertices[i + 23] = p00[1];
      }
    }

    assert(m_vertex_buffer.create());

    assert(m_vertex_buffer.bind());

    m_vertex_buffer.allocate(&vertices[0], value_count * sizeof(float));
  }

  void InitRenderRequests(IDGenerator& id_gen)
  {
    for (int i = 0; i < m_partition_level; i++) {

      const int j_max = m_partition_level * m_partition_level;

      const int j_stride = j_max / ((i + 1) * (i + 1));

      for (int j = 0; j < j_max; j += j_stride) {

        RenderRequest req{};

        req.unique_id = id_gen.Generate();

        req.partition_index = i;

        req.x_pixel_count = m_partition_width;
        req.y_pixel_count = m_partition_height;

        req.x_pixel_stride = m_partition_level;
        req.y_pixel_stride = m_partition_level;

        req.x_pixel_offset = ReverseInterleaveX(j);
        req.y_pixel_offset = ReverseInterleaveY(j);

        req.x_frame_size = m_width;
        req.y_frame_size = m_height;

        if (!ContainsRenderRequest(req))
          m_render_requests.emplace_back(req);
      }
    }
  }

  bool ContainsRenderRequest(const RenderRequest& req) const
  {
    for (const auto& existing_req : m_render_requests) {
      if ((req.x_pixel_offset == existing_req.x_pixel_offset) &&
          (req.y_pixel_offset == existing_req.y_pixel_offset))
        return true;
    }

    return false;
  }

private:
  int m_width;

  int m_height;

  int m_padded_width;

  int m_padded_height;

  int m_partition_level;

  int m_partition_width;

  int m_partition_height;

  QOpenGLBuffer m_vertex_buffer{ QOpenGLBuffer::VertexBuffer };

  std::vector<RenderRequest> m_render_requests;

  std::vector<std::unique_ptr<RenderReply>> m_render_replies;

  size_t m_render_request_index = 0;

  size_t m_render_reply_index = 0;

  int m_partition_index = -1;
};

class ViewImpl : public View
{
public:
  ViewImpl(QWidget* parent)
    : View(parent)
  {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setFocusPolicy(Qt::StrongFocus);
  }

  ~ViewImpl() { makeCurrent(); }

  void SetPartitionLevel(int level) override
  {
    level = std::max(level, 0);
    level = std::min(level, 16);
    m_partition_level = level;
  }

  void NewFrame() override
  {
    makeCurrent();

    m_frame_build_context.reset(
      new FrameBuildContext(size(), m_partition_level, m_id_generator));

    doneCurrent();
  }

  bool NeedsNewFrame() override
  {
    if (!m_frame_build_context)
      return true;

    QSize widget_size = size();

    return m_frame_build_context->HasParameters(widget_size, m_partition_level);
  }

  const RenderRequest* PopRenderRequest() override
  {
    if (!m_frame_build_context)
      return nullptr;
    else
      return m_frame_build_context->PopRenderRequest();
  }

  void ReplyRenderRequest(const RenderRequest* req,
                          std::vector<unsigned char>&& data) override
  {
    if (data.size() != size_t(req->x_pixel_count * req->y_pixel_count * 3))
      return;

    makeCurrent();

    if (m_frame_build_context->ReplyRenderRequest(req, std::move(data)))
      update();

    doneCurrent();
  }

protected:
  void initializeGL() override
  {
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    m_program.link();

    NewFrame();
  }

  void paintGL() override
  {
    if (!m_frame_build_context)
      return;

    QOpenGLFunctions* functions = QOpenGLContext::currentContext()->functions();

    // Clear Window

    functions->glClearColor(0, 0, 0.1, 1);

    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int reply_index = m_frame_build_context->GetReplyCount() - 1;
    if (reply_index < 0)
      return;

    //

    QOpenGLTexture* texture = m_frame_build_context->GetTexture(reply_index);

    const RenderRequest* req =
      m_frame_build_context->GetRenderRequest(reply_index);

    texture->bind();

    //

    assert(m_program.bind());

    //

    assert(m_frame_build_context->GetVertexBuffer().bind());

    const int position_attrib = m_program.attributeLocation("position");

    const float u_offset = req->x_pixel_offset / float(req->x_frame_size);
    const float v_offset = req->y_pixel_offset / float(req->y_frame_size);

    m_program.setUniformValue("u_offset", u_offset);
    m_program.setUniformValue("v_offset", v_offset);

    const float u_stride = float(req->x_pixel_stride);
    const float v_stride = float(req->y_pixel_stride);

    m_program.setUniformValue("u_stride", u_stride);
    m_program.setUniformValue("v_stride", v_stride);

    m_program.enableAttributeArray(position_attrib);

    functions->glVertexAttribPointer(
      position_attrib, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);

    const int v_count = m_frame_build_context->GetVertexCount();

    functions->glDrawArrays(GL_TRIANGLES, 0, v_count);

    m_program.disableAttributeArray(position_attrib);

    qDebug() << "Frame rendered.";
  }

  void resizeGL(int w, int h) override
  {
    context()->functions()->glViewport(0, 0, w, h);
  }

private:
  std::unique_ptr<FrameBuildContext> m_frame_build_context;

  IDGenerator m_id_generator;

  QOpenGLShaderProgram m_program;

  int m_partition_level = 4;
};

} // namespace

View*
CreateView(QWidget* parent)
{
  return new ViewImpl(parent);
}

} // namespace vision::gui
