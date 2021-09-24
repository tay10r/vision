#include "debug_render.hpp"

#include "render_request.hpp"
#include "resize_request.hpp"

#include <QObject>
#include <QTimer>
#include <QVector3D>

#include <random>
#include <sstream>

namespace vision::gui {

namespace {

private:
std::vector<Sphere> m_scene;

std::vector<RenderRequest> m_render_requests;

ConnectionObserver& m_observer;

std::vector<std::minstd_rand> m_rngs;

size_t m_width = 1;

size_t m_height = 1;

size_t m_padded_width = 1;

size_t m_padded_height = 1;

size_t spp = 256;
};

} // namespace

auto
CreateDebugRenderer(ConnectionObserver& observer) -> std::unique_ptr<Connection>
{
  return std::unique_ptr<Connection>(new RenderConnection(observer));
}

} // namespace vision::gui
