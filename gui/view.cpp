#include "view.hpp"

#include "schedule.hpp"
#include "vertex.hpp"

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <map>
#include <optional>

#include <assert.h>
#include <stdint.h>

namespace vision::gui {

namespace {

const char* vertSrc = R"(
#version 330 core

layout(location = 0) in vec4 vertex;

uniform float x_partition_size = 0.0;
uniform float y_partition_size = 0.0;

uniform float x_pixel_offset = 0.0;
uniform float y_pixel_offset = 0.0;

uniform float x_pixel_stride = 1.0;
uniform float y_pixel_stride = 1.0;

out vec2 tex_coords;

void
main()
{
  float x_max = x_partition_size * x_pixel_stride;
  float y_max = y_partition_size * y_pixel_stride;

  vec2 offset = vec2(0.0, 0.0);

  offset += vec2(vertex.x * x_partition_size * x_pixel_stride,
                 vertex.y * y_partition_size * y_pixel_stride);

  offset += vec2(vertex.z * x_partition_size,
                 vertex.w * y_partition_size);

  offset += vec2(x_pixel_offset, y_pixel_offset);

  tex_coords = vertex.xy;

  vec2 position = vec2(offset.x / x_max,
                       offset.y / y_max);

  float ndc_x = (position.x * 2.0) - 1.0;
  float ndc_y = 1.0 - (position.y * 2.0);
  gl_Position = vec4(ndc_x, ndc_y, 0.0, 1.0);
}
)";

const char* fragSrc = R"(
#version 330 core

out vec4 color;

uniform sampler2D partition;

in vec2 tex_coords;

void
main()
{
  color = texture(partition, tex_coords);
}
)";

struct RenderReply final
{
  QOpenGLTexture texture{ QOpenGLTexture::Target2D };

  RenderReply(const RenderRequest& req, const unsigned char* data)
    : texture(QImage(data,
                     req.x_pixel_count,
                     req.y_pixel_count,
                     req.x_pixel_count * 3,
                     QImage::Format_RGB888),
              QOpenGLTexture::DontGenerateMipMaps)
  {
    texture.setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Nearest);
  }
};

class FrameBuildContext final
{
public:
  FrameBuildContext(const QSize& size, size_t div_level)
    : FrameBuildContext(size.width(), size.height(), div_level)
  {}

  FrameBuildContext(size_t w, size_t h, size_t div_level)
    : m_schedule(w, h, div_level)
    , m_vertex_buffer(QOpenGLBuffer::VertexBuffer)
  {
    InitVertexBuffer();
  }

  size_t GetVertexCount() const
  {
    const size_t x_max = m_schedule.GetTextureWidth() - 1;
    const size_t y_max = m_schedule.GetTextureHeight() - 1;
    return x_max * y_max * 6;
  }

  QOpenGLBuffer& GetVertexBuffer() { return m_vertex_buffer; }

  RenderRequest GetRenderRequest() { return m_schedule.GetRenderRequest(); }

  const Schedule& GetSchedule() const { return m_schedule; }

  bool ReplyRenderRequest(const RenderRequest& req, const unsigned char* data)
  {
    if (m_render_replies.size() >= m_schedule.GetRenderRequestCount())
      return false;

    m_render_replies.emplace_back(new RenderReply(req, data));

    const std::optional<size_t> m_last_preview_index = m_preview_index;

    m_schedule.NextRenderRequest();

    m_preview_index = m_schedule.GetPreviewIndex();

    return m_preview_index != m_last_preview_index;
  }

  size_t GetReplyCount() { return m_render_replies.size(); }

  QOpenGLTexture* GetTexture(size_t index)
  {
    return &m_render_replies.at(index)->texture;
  }

private:
  void InitVertexBuffer()
  {
    std::vector<Vertex> vertices = m_schedule.GetVertexBuffer();

    assert(m_vertex_buffer.create());

    assert(m_vertex_buffer.bind());

    m_vertex_buffer.allocate(&vertices[0],
                             vertices.size() * sizeof(vertices[0]));
  }

private:
  Schedule m_schedule;

  std::optional<size_t> m_preview_index;

  QOpenGLBuffer m_vertex_buffer{ QOpenGLBuffer::VertexBuffer };

  std::vector<std::unique_ptr<RenderReply>> m_render_replies;
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

  bool HasRenderRequest() const override
  {
    if (!m_frame_build_context)
      return false;

    const Schedule& schedule = m_frame_build_context->GetSchedule();

    return schedule.GetRemainingRenderRequests();
  }

  void SetDivisionLevel(size_t level) override
  {
    level = std::max(level, size_t(0));
    level = std::min(level, size_t(4));
    m_div_level = level;
  }

  void NewFrame() override
  {
    makeCurrent();

    m_frame_build_context.reset(new FrameBuildContext(size(), m_div_level));

    doneCurrent();
  }

  bool NeedsNewFrame() override { return false; }

  RenderRequest GetCurrentRenderRequest() const override
  {
    if (!m_frame_build_context)
      return RenderRequest{};
    else
      return m_frame_build_context->GetRenderRequest();
  }

  bool ReplyRenderRequest(const unsigned char* data, size_t size) override
  {
    makeCurrent();

    const RenderRequest req = GetCurrentRenderRequest();
    if (!req.IsValid())
      return false;

    size_t req_size = req.x_pixel_count * req.y_pixel_count * 3;

    if (req_size != size)
      return false;

    if (m_frame_build_context->ReplyRenderRequest(req, data))
      update();

    doneCurrent();

    return true;
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

    const Schedule& schedule = m_frame_build_context->GetSchedule();

    QOpenGLFunctions* functions = QOpenGLContext::currentContext()->functions();

    // Clear Window

    functions->glClearColor(0, 0, 0, 1);

    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<PreviewOperation> preview_operations =
      schedule.GetPreviewOperations();

    const int vertex_attrib = m_program.attributeLocation("vertex");

    assert(m_program.bind());

    assert(m_frame_build_context->GetVertexBuffer().bind());

    m_program.enableAttributeArray(vertex_attrib);

    functions->glVertexAttribPointer(
      vertex_attrib, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);

    const size_t partition_w = schedule.GetPartitionWidth();
    const size_t partition_h = schedule.GetPartitionHeight();

    m_program.setUniformValue("x_partition_size", float(partition_w));
    m_program.setUniformValue("y_partition_size", float(partition_h));

    for (size_t i = 0; i < preview_operations.size(); i++) {

      QOpenGLTexture* texture = m_frame_build_context->GetTexture(i);

      texture->bind();

      const PreviewOperation& op = preview_operations.at(i);

      m_program.setUniformValue("x_pixel_offset", float(op.x_pixel_offset));
      m_program.setUniformValue("y_pixel_offset", float(op.y_pixel_offset));

      m_program.setUniformValue("x_pixel_stride", float(op.x_pixel_stride));
      m_program.setUniformValue("y_pixel_stride", float(op.y_pixel_stride));

      const int v_count = m_frame_build_context->GetVertexCount();

      functions->glDrawArrays(GL_TRIANGLES, 0, v_count);
    }

    m_program.disableAttributeArray(vertex_attrib);
  }

  void resizeGL(int w, int h) override
  {
    context()->functions()->glViewport(0, 0, w, h);

    m_frame_build_context.reset(
      new FrameBuildContext(QSize(w, h), m_div_level));
  }

private:
  std::unique_ptr<FrameBuildContext> m_frame_build_context;

  QOpenGLShaderProgram m_program;

  size_t m_div_level = 4;
};

} // namespace

View*
CreateView(QWidget* parent)
{
  return new ViewImpl(parent);
}

} // namespace vision::gui
