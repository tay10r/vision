#include "view.hpp"

#include "id_generator.hpp"
#include "resize_request.hpp"
#include "schedule.hpp"
#include "vertex.hpp"

#include <QKeyEvent>
#include <QMouseEvent>
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

#ifdef NDEBUG
#undef assert
#define assert(expr) ((void)expr)
#endif

namespace vision::gui {

namespace {

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
  FrameBuildContext(const QSize& size,
                    size_t div_level,
                    IDGenerator& id_generator)
    : FrameBuildContext(size.width(), size.height(), div_level, id_generator)
  {}

  FrameBuildContext(size_t w,
                    size_t h,
                    size_t div_level,
                    IDGenerator& id_generator)
    : m_schedule(w, h, div_level, id_generator)
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

  ResizeRequest MakeResizeRequest() const
  {
    return ResizeRequest{ m_schedule.GetFrameWidth(),
                          m_schedule.GetFrameHeight(),
                          m_schedule.GetTextureWidth(),
                          m_schedule.GetTextureHeight() };
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

    bool success = m_vertex_buffer.create();

    assert(success);

    success = m_vertex_buffer.bind();

    assert(success);

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

    setMouseTracking(true);
  }

  ~ViewImpl() { makeCurrent(); }

  auto GetSchedule() const -> const Schedule* override
  {
    if (m_frame_build_context)
      return &m_frame_build_context->GetSchedule();
    else
      return nullptr;
  }

  ResizeRequest MakeResizeRequest() const override
  {
    if (!m_frame_build_context)
      return ResizeRequest{};

    return m_frame_build_context->MakeResizeRequest();
  }

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

    m_frame_build_context.reset(
      new FrameBuildContext(size(), m_div_level, m_id_generator));

    doneCurrent();

    NotifyNewFrame();
  }

  bool NeedsNewFrame() override { return false; }

  RenderRequest GetCurrentRenderRequest() const override
  {
    if (!m_frame_build_context)
      return RenderRequest{};
    else
      return m_frame_build_context->GetRenderRequest();
  }

  bool ReplyRenderRequest(const unsigned char* data,
                          size_t size,
                          size_t request_id) override
  {
    makeCurrent();

    const RenderRequest req = GetCurrentRenderRequest();
    if (!req.IsValid())
      return false;

    if (req.id != request_id)
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
  void focusInEvent(QFocusEvent* event) override
  {
    setCursor(Qt::BlankCursor);

    QOpenGLWidget::focusInEvent(event);
  }

  void focusOutEvent(QFocusEvent* event) override
  {
    unsetCursor();

    QOpenGLWidget::focusOutEvent(event);
  }

  void mouseMoveEvent(QMouseEvent* event) override
  {
    if (hasFocus())
      NotifyMouseMoveEvent(event->x(), event->y());

    QWidget::mouseMoveEvent(event);
  }

  void keyPressEvent(QKeyEvent* event) override
  {
    QString key_text = event->text();

    if (!key_text.isEmpty() && !event->isAutoRepeat())
      NotifyKeyEvent(key_text, true);

    QOpenGLWidget::keyPressEvent(event);
  }

  void keyReleaseEvent(QKeyEvent* event) override
  {
    QString key_text = event->text();

    if (!key_text.isEmpty() && !event->isAutoRepeat())
      NotifyKeyEvent(key_text, false);

    QOpenGLWidget::keyPressEvent(event);
  }

  QString GetButtonName(const QMouseEvent* event)
  {
    switch (event->button()) {
      case Qt::LeftButton:
        return "left";
      case Qt::RightButton:
        return "right";
      default:
        break;
    }

    return QString();
  }

  bool NotifyMouseButtonEvent(const QMouseEvent* event, bool state)
  {
    const QString button_name = GetButtonName(event);

    if (button_name.isEmpty())
      return false;

    View::NotifyMouseButtonEvent(button_name, event->x(), event->y(), state);

    return true;
  }

  void mousePressEvent(QMouseEvent* event) override
  {
    NotifyMouseButtonEvent(event, true);

    QOpenGLWidget::mousePressEvent(event);
  }

  void mouseReleaseEvent(QMouseEvent* event) override
  {
    NotifyMouseButtonEvent(event, false);

    QOpenGLWidget::mouseReleaseEvent(event);
  }

  void initializeGL() override
  {
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                      ":/shaders/blit_partition.vert");

    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                      ":/shaders/blit_partition.frag");

    m_program.link();

    NewFrame();
  }

  void paintGL() override
  {
    if (!m_frame_build_context)
      return;

    const Schedule& schedule = m_frame_build_context->GetSchedule();

    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (!context)
      return;

    QOpenGLFunctions* functions = context->functions();
    if (!functions)
      return;

    // Clear Window

    functions->glClearColor(0, 0, 0, 1);

    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<PreviewOperation> preview_operations =
      schedule.GetPreviewOperations();

    const int vertex_attrib = m_program.attributeLocation("vertex");

    bool success = m_program.bind();

    assert(success);

    success = m_frame_build_context->GetVertexBuffer().bind();

    assert(success);

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

    NotifyResize();

    NewFrame();
  }

private:
  std::unique_ptr<FrameBuildContext> m_frame_build_context;

  QOpenGLShaderProgram m_program;

  size_t m_div_level = 3;

  IDGenerator m_id_generator;
};

} // namespace

View*
CreateView(QWidget* parent)
{
  return new ViewImpl(parent);
}

void
View::NotifyResize()
{
  const ResizeRequest req = MakeResizeRequest();

  if (m_observer) {
    m_observer->OnResize(
      req.width, req.height, req.padded_width, req.padded_height);
  }
}

void
View::NotifyKeyEvent(const QString& key_text, bool state)
{
  if (m_observer)
    m_observer->OnKeyEvent(key_text, state);
}

void
View::NotifyMouseButtonEvent(const QString& button_name,
                             int x,
                             int y,
                             bool state)
{
  if (m_observer)
    m_observer->OnMouseButtonEvent(button_name, x, y, state);
}

void
View::NotifyMouseMoveEvent(int x, int y)
{
  if (m_observer)
    m_observer->OnMouseMoveEvent(x, y);
}

void
View::NotifyNewFrame()
{
  const Schedule* schedule = GetSchedule();

  if (schedule && m_observer)
    m_observer->OnNewFrame(*schedule);
}

} // namespace vision::gui
