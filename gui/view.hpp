#pragma once

#include <QOpenGLWidget>

#include <vector>

#include <stddef.h>

namespace vision::gui {

struct RenderRequest;

class View : public QOpenGLWidget
{
public:
  View(QWidget* parent)
    : QOpenGLWidget(parent)
  {}

  virtual ~View() = default;

  /// Gets the current render request made by the view.
  ///
  /// @return The current render request.
  virtual RenderRequest GetCurrentRenderRequest() const = 0;

  /// Responds to a render request with the resultant RGB buffer.
  ///
  /// @param req The request that the reply is for.
  ///
  /// @param buffer The buffer containing the 24-bit RGB buffer. Should fit all
  ///               24-bit RGB values requested.
  virtual void ReplyRenderRequest(const RenderRequest& req,
                                  const unsigned char* data) = 0;

  virtual void NewFrame() = 0;

  virtual void SetDivisionLevel(size_t level) = 0;

  /// Indicates whether or not the view needs to go through the rendering
  /// process again. This can return true if the partition level is changed or
  /// if the window is resized.
  virtual bool NeedsNewFrame() = 0;
};

View*
CreateView(QWidget* parent);

} // namespace vision::gui
