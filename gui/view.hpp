#pragma once

#include <QOpenGLWidget>

#include <vector>

namespace vision::gui {

struct RenderRequest final
{
  int unique_id = 0;

  int partition_index = 0;

  int x_pixel_count = 0;
  int y_pixel_count = 0;

  int x_pixel_offset = 0;
  int y_pixel_offset = 0;

  int x_pixel_stride = 0;
  int y_pixel_stride = 0;

  int x_frame_size = 0;
  int y_frame_size = 0;
};

class View : public QOpenGLWidget
{
public:
  View(QWidget* parent)
    : QOpenGLWidget(parent)
  {}

  virtual ~View() = default;

  /// Handles a render request made by the view.
  ///
  /// @return A pointer to the render request. If the view is already fully
  ///         rendered, then this function returns a null pointer.
  virtual const RenderRequest* PopRenderRequest() = 0;

  /// Responds to a render request with the resultant RGB buffer.
  ///
  /// @param req The request that the reply is for.
  ///
  /// @param buffer The buffer containing the 24-bit RGB buffer. Should fit all
  ///               24-bit RGB values requested.
  virtual void ReplyRenderRequest(const RenderRequest* req,
                                  std::vector<unsigned char>&& buffer) = 0;

  virtual void NewFrame() = 0;

  virtual void SetPartitionLevel(int level) = 0;

  /// Indicates whether or not the view needs to go through the rendering
  /// process again. This can return true if the partition level is changed or
  /// if the window is resized.
  virtual bool NeedsNewFrame() = 0;
};

View*
CreateView(QWidget* parent);

} // namespace vision::gui