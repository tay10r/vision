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

struct Ray final
{
  QVector3D org;
  QVector3D dir;
};

struct Hit final
{
  float distance = std::numeric_limits<float>::infinity();

  QVector3D position;

  QVector3D normal;

  QVector3D diffuse;

  QVector3D emission;

  operator bool() const noexcept
  {
    return distance != std::numeric_limits<float>::infinity();
  }
};

struct Sphere final
{
  QVector3D diffuse;

  QVector3D emission;

  QVector3D center;

  float radius;

  float GetSquaredRadius() const noexcept { return radius * radius; }
};

Hit
GetHitInfo(const Ray& ray, const Sphere& sphere, float distance)
{
  const QVector3D hit_pos = ray.org + (ray.dir * distance);

  const QVector3D hit_norm = (hit_pos - sphere.center) / sphere.radius;

  return Hit{ distance, hit_pos, hit_norm, sphere.diffuse, sphere.emission };
}

float
Intersect(const Ray& ray, const Sphere& sphere)
{
  const QVector3D oc = ray.org - sphere.center;
  const float a = QVector3D::dotProduct(ray.dir, ray.dir);
  const float half_b = QVector3D::dotProduct(oc, ray.dir);
  const float c = QVector3D::dotProduct(oc, oc) - sphere.GetSquaredRadius();
  const float discriminant = (half_b * half_b) - (a * c);

  if (discriminant < 0)
    return std::numeric_limits<float>::infinity();

  const float disc_root = std::sqrt(discriminant);

  const float t0 = (-half_b - disc_root) / a;
  const float t1 = (-half_b + disc_root) / a;

  const float t_min = std::min(t0, t1);
  const float t_max = std::max(t0, t1);

  if (t_min > 0)
    return t_min;
  else if (t_max > 0)
    return t_max;
  else
    return std::numeric_limits<float>::infinity();
}

class RenderConnection final
  : public Connection
  , public QObject
{
public:
  RenderConnection(ConnectionObserver& o)
    : m_observer(o)
  {
    GenerateScene();
  }

  bool Connect() override
  {
    m_observer.OnConnectionStart();

    return true;
  }

  void Render(const RenderRequest& req) override
  {
    m_render_requests.emplace_back(req);

    QTimer::singleShot(0, this, &RenderConnection::HandleFirstRenderRequest);
  }

  void Resize(const ResizeRequest& req) override
  {
    m_width = std::max(req.width, size_t(1));

    m_height = std::max(req.height, size_t(1));

    m_padded_width = std::max(req.padded_width, size_t(1));

    m_padded_height = std::max(req.padded_height, size_t(1));

    m_rngs.clear();

    std::mt19937 seed_rng(1234 * req.padded_width * req.padded_height);

    for (size_t i = 0; i < (req.padded_width * req.padded_height); i++)
      m_rngs.emplace_back(seed_rng());
  }

  void SendKey(const std::string_view&, bool) override {}

  void SendMouseButton(const std::string_view&, int, int, bool) override {}

  void SendMouseMove(int, int) override {}

protected slots:
  void HandleFirstRenderRequest()
  {
    if (m_render_requests.empty())
      return;

    RenderRequest req = m_render_requests[0];

    // Get rid of render requests issued for a different size.

    while ((req.x_frame_size != m_width) || (req.y_frame_size != m_height)) {

      m_render_requests.erase(m_render_requests.begin());

      if (m_render_requests.empty())
        return;

      req = m_render_requests[0];
    }

    HandleRenderRequest(req);

    m_render_requests.erase(m_render_requests.begin());
  }

private:
  std::minstd_rand& GetRng(size_t x, size_t y)
  {
    return m_rngs[(y * m_padded_width) + x];
  }

  Ray GenerateRay(size_t x, size_t y)
  {
    const float u_min = (x + 0.0f) / m_width;
    const float v_min = (y + 0.0f) / m_height;

    const float u_max = (x + 1.0f) / m_width;
    const float v_max = (y + 1.0f) / m_height;

    std::uniform_real_distribution<float> u_dist(u_min, u_max);
    std::uniform_real_distribution<float> v_dist(v_min, v_max);

    auto& rng = GetRng(x, y);

    const float u = u_dist(rng);
    const float v = v_dist(rng);

    const float aspect = float(m_width) / m_height;

    const float dx = ((u * 2) - 1) * aspect;
    const float dy = (1 - (v * 2));
    const float dz = -1;

    const QVector3D org(0, 0, 0);

    const QVector3D dir(QVector3D(dx, dy, dz).normalized());

    return Ray{ org, dir };
  }

  QVector3D OnMiss(const Ray& ray) const
  {
    const QVector3D up(0, 1, 0);

    const float level = (QVector3D::dotProduct(ray.dir, up) + 1.0f) * 0.5f;

    const QVector3D lo(0.047, 0.078, 0.271);

    const QVector3D hi(0.22, 0.157, 0.361);

    return lo + ((hi - lo) * level);
  }

  template<typename Rng>
  QVector3D Trace(const Ray& ray, Rng& rng, size_t depth = 0)
  {
    Hit hit = IntersectScene(ray);
    if (!hit)
      return OnMiss(ray);
    else
      return Shade(ray, hit, rng, depth);
  }

  template<typename Rng>
  QVector3D Shade(const Ray& ray, const Hit& hit, Rng& rng, size_t depth)
  {
    if (depth > 2)
      return hit.emission;

    const float shadow_bias = 0.0001;

    const QVector3D hit_pos =
      ray.org + (ray.dir * (hit.distance - shadow_bias));

    const Ray second_ray{ hit_pos, SampleHemisphere(hit.normal, rng) };

    return (hit.diffuse * Trace(second_ray, rng, depth + 1)) + hit.emission;
  }

  Hit IntersectScene(const Ray& ray) const
  {
    Hit closest;

    for (const Sphere& s : m_scene) {

      const float distance = Intersect(ray, s);

      if (distance < closest.distance)
        closest = GetHitInfo(ray, s, distance);
    }

    return closest;
  }

  template<typename Rng>
  QVector3D SampleUnitSphere(Rng& rng)
  {
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (size_t i = 0; i < 128; i++) {

      QVector3D v(dist(rng), dist(rng), dist(rng));

      if (QVector3D::dotProduct(v, v) <= 1)
        return v.normalized();
    }

    return QVector3D(0, 1, 0);
  }

  template<typename Rng>
  QVector3D SampleHemisphere(const QVector3D& dir, Rng& rng)
  {
    for (size_t i = 0; i < 128; i++) {

      QVector3D v = SampleUnitSphere(rng);

      if (QVector3D::dotProduct(v, dir) >= 0)
        return v;
    }

    return dir;
  }

  QVector3D RenderPixel(size_t x, size_t y)
  {
    auto& rng = GetRng(x, y);

    QVector3D color(0, 0, 0);

    for (size_t i = 0; i < spp; i++) {

      const Ray primary_ray = GenerateRay(x, y);

      color = color + Trace(primary_ray, rng);
    }

    return color * (1.0f / spp);
  }

  void HandleRenderRequest(const RenderRequest& req)
  {
    std::vector<unsigned char> buffer(req.x_pixel_count * req.y_pixel_count *
                                      3);

#pragma omp parallel for

    for (long y = 0; y < long(req.y_pixel_count); y++) {

      for (size_t x = 0; x < req.x_pixel_count; x++) {

        size_t frame_x = req.GetFrameX(x);
        size_t frame_y = req.GetFrameY(y);

        const QVector3D color = RenderPixel(frame_x, frame_y);

        size_t buffer_offset = ((y * req.x_pixel_count) + x) * 3;

        buffer[buffer_offset + 0] = color[0] * 255;
        buffer[buffer_offset + 1] = color[1] * 255;
        buffer[buffer_offset + 2] = color[2] * 255;
      }
    }

    std::ostringstream header_stream;

    header_stream << "rgb buffer " << req.x_pixel_count << ' '
                  << req.y_pixel_count << '\n';

    std::string header = header_stream.str();

    m_observer.OnConnectionRecv((const unsigned char*)&header[0],
                                header.size());

    m_observer.OnConnectionRecv(&buffer[0], buffer.size());
  }

  void GenerateScene()
  {
    m_scene.emplace_back(Sphere{ QVector3D(0.8f, 0.8f, 0.8f),
                                 QVector3D(0.0f, 0.0f, 0.0f),
                                 QVector3D(-1.0f, 0.0f, -5.0f),
                                 0.5f });

    m_scene.emplace_back(Sphere{ QVector3D(0.0f, 0.0f, 0.0f),
                                 QVector3D(1.0f, 1.0f, 0.0f),
                                 QVector3D(0.0f, 0.0f, -5.0f),
                                 0.5f });

    m_scene.emplace_back(Sphere{ QVector3D(0.8f, 0.8f, 0.8f),
                                 QVector3D(0.0f, 0.0f, 0.0f),
                                 QVector3D(1.0f, 0.0f, -5.0f),
                                 0.5f });

    m_scene.emplace_back(Sphere{ QVector3D(0.8f, 0.8f, 0.8f),
                                 QVector3D(0.0f, 0.0f, 0.0f),
                                 QVector3D(0.0f, -100.5f, -5.0f),
                                 100.0f });
  }

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
