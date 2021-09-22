#pragma once

#include <QOpenGLWidget>

#include <vector>

#include <stddef.h>

class QString;

namespace vision::gui {

struct RenderRequest;
struct ResizeRequest;

class ViewObserver
{
public:
  virtual ~ViewObserver() = default;

  virtual void OnViewResize(size_t w,
                            size_t h,
                            size_t padded_w,
                            size_t padded_h) = 0;

  virtual void OnViewKeyEvent(const QString& key, bool state) = 0;

  virtual void OnViewMouseButtonEvent(const QString& button,
                                      int x,
                                      int y,
                                      bool state) = 0;

  virtual void OnViewMouseMoveEvent(int x, int y) = 0;

  virtual void OnNewViewFrame() = 0;
};

class View : public QOpenGLWidget
{
public:
  View(ViewObserver& observer, QWidget* parent)
    : QOpenGLWidget(parent)
    , m_observer(observer)
  {}

  virtual ~View() = default;

  virtual ResizeRequest MakeResizeRequest() const = 0;

  virtual bool HasRenderRequest() const = 0;

  /// Gets the current render request made by the view.
  ///
  /// @return The current render request.
  virtual RenderRequest GetCurrentRenderRequest() const = 0;

  /// Responds to the current render request with the resultant RGB buffer.
  ///
  /// @param data The buffer containing the 24-bit RGB buffer. Should fit all
  ///             24-bit RGB values requested.
  ///
  /// @param size The number of bytes in the data buffer.
  ///
  /// @param request_id The ID of the render request that this reply is for.
  ///
  /// @return True on success, false on failure.
  virtual bool ReplyRenderRequest(const unsigned char* data,
                                  size_t size,
                                  size_t request_id) = 0;

  virtual void NewFrame() = 0;

  virtual void SetDivisionLevel(size_t level) = 0;

  /// Indicates whether or not the view needs to go through the rendering
  /// process again. This can return true if the partition level is changed or
  /// if the window is resized.
  virtual bool NeedsNewFrame() = 0;

protected:
  void NotifyResize();

  void NotifyKeyEvent(const QString& key_text, bool state);

  void NotifyMouseButtonEvent(const QString& button_name,
                              int x,
                              int y,
                              bool state);

  void NotifyMouseMoveEvent(int x, int y);

  void NotifyNewFrame();

private:
  ViewObserver& m_observer;
};

View*
CreateView(ViewObserver& observer, QWidget* parent);

} // namespace vision::gui
