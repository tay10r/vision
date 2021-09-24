#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <stdlib.h>

#include <QVector3D>

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

class PathTracer final
{
public:
  PathTracer();

  auto Render(int x_count,
              int y_count,
              int x_offset,
              int y_offset,
              int x_stride,
              int y_stride) -> std::vector<unsigned char>;

  void HandleResize(int w, int h, int padded_w, int padded_h);

protected:
  Ray GenerateRay(int x, int y);

  QVector3D OnMiss(const Ray& ray) const;

  Hit IntersectScene(const Ray& ray) const;

  template<typename Rng>
  QVector3D Trace(const Ray& ray, Rng& rng, int depth = 0);

  template<typename Rng>
  QVector3D Shade(const Ray& ray, const Hit& hit, Rng& rng, int depth);

  template<typename Rng>
  static QVector3D SampleHemisphere(const QVector3D& dir, Rng& rng);

  template<typename Rng>
  static QVector3D SampleUnitSphere(Rng& rng);

  QVector3D RenderPixel(int x, int y);

private:
  int m_frame_w = 0;
  int m_frame_h = 0;

  int m_padded_w = 0;
  int m_padded_h = 0;

  int m_spp = 16;

  std::vector<Sphere> m_scene;

  std::vector<std::minstd_rand> m_rngs;
};

void
PathTracer::HandleResize(int w, int h, int padded_w, int padded_h)
{
  m_frame_w = w;
  m_frame_h = h;
  m_padded_w = padded_w;
  m_padded_h = padded_h;

  std::mt19937 seed_rng(1234 * padded_w * padded_h);

  m_rngs.clear();

  for (int i = 0; i < (padded_w * padded_h); i++)
    m_rngs.emplace_back(seed_rng());
}

Ray
PathTracer::GenerateRay(int x, int y)
{
  const float u_min = (x + 0.0f) / m_frame_w;
  const float v_min = (y + 0.0f) / m_frame_h;

  const float u_max = (x + 1.0f) / m_frame_w;
  const float v_max = (y + 1.0f) / m_frame_h;

  std::uniform_real_distribution<float> u_dist(u_min, u_max);
  std::uniform_real_distribution<float> v_dist(v_min, v_max);

  auto& rng = m_rngs[(y * m_padded_w) + x];

  const float u = u_dist(rng);
  const float v = v_dist(rng);

  const float aspect = float(m_frame_w) / m_frame_h;

  const float dx = ((u * 2) - 1) * aspect;
  const float dy = (1 - (v * 2));
  const float dz = -1;

  const QVector3D org(0, 0, 0);

  const QVector3D dir(QVector3D(dx, dy, dz).normalized());

  return Ray{ org, dir };
}

QVector3D
PathTracer::OnMiss(const Ray& ray) const
{
  const QVector3D up(0, 1, 0);

  const float level = (QVector3D::dotProduct(ray.dir, up) + 1.0f) * 0.5f;

  const QVector3D lo(0.047, 0.078, 0.271);

  const QVector3D hi(0.22, 0.157, 0.361);

  return lo + ((hi - lo) * level);
}

template<typename Rng>
QVector3D
PathTracer::Trace(const Ray& ray, Rng& rng, int depth)
{
  Hit hit = IntersectScene(ray);
  if (!hit)
    return OnMiss(ray);
  else
    return Shade(ray, hit, rng, depth);
}

template<typename Rng>
QVector3D
PathTracer::Shade(const Ray& ray, const Hit& hit, Rng& rng, int depth)
{
  if (depth > 2)
    return hit.emission;

  const float shadow_bias = 0.0001;

  const QVector3D hit_pos = ray.org + (ray.dir * (hit.distance - shadow_bias));

  const Ray second_ray{ hit_pos, SampleHemisphere(hit.normal, rng) };

  return (hit.diffuse * Trace(second_ray, rng, depth + 1)) + hit.emission;
}

Hit
PathTracer::IntersectScene(const Ray& ray) const
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
QVector3D
PathTracer::SampleUnitSphere(Rng& rng)
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
QVector3D
PathTracer::SampleHemisphere(const QVector3D& dir, Rng& rng)
{
  for (size_t i = 0; i < 128; i++) {

    QVector3D v = SampleUnitSphere(rng);

    if (QVector3D::dotProduct(v, dir) >= 0)
      return v;
  }

  return dir;
}

QVector3D
PathTracer::RenderPixel(int x, int y)
{
  auto& rng = m_rngs[(y * m_padded_w) + x];

  QVector3D color(0, 0, 0);

  for (size_t i = 0; i < m_spp; i++) {

    const Ray primary_ray = GenerateRay(x, y);

    color = color + Trace(primary_ray, rng);
  }

  return color * (1.0f / m_spp);
}

auto
PathTracer::Render(int x_count,
                   int y_count,
                   int x_offset,
                   int y_offset,
                   int x_stride,
                   int y_stride) -> std::vector<unsigned char>
{
  std::vector<unsigned char> buffer(x_count * y_count * 3);

#pragma omp parallel for

  for (int y = 0; y < y_count; y++) {

    for (int x = 0; x < x_count; x++) {

      const int abs_x = (x * x_stride) + x_offset;
      const int abs_y = (y * y_stride) + y_offset;

      const QVector3D color = RenderPixel(abs_x, abs_y);

      size_t buffer_offset = ((y * x_count) + x) * 3;

      buffer[buffer_offset + 0] = color[0] * 255;
      buffer[buffer_offset + 1] = color[1] * 255;
      buffer[buffer_offset + 2] = color[2] * 255;
    }
  }

  return buffer;
}

PathTracer::PathTracer()
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

int
main()
{
  int i = 0;

  PathTracer path_tracer;

  int w = 0;
  int h = 0;
  int padded_w = 0;
  int padded_h = 0;

  int x_count = 0;
  int y_count = 0;
  int x_offset = 0;
  int y_offset = 0;
  int x_stride = 0;
  int y_stride = 0;
  int render_request_id = 0;

  while (std::cin) {

    std::string command;

    std::getline(std::cin, command);

    std::cerr << "command: " << command << std::endl;

    if (command.empty() || (command == "q"))
      break;

    switch (command[0]) {
      case 'r':
        sscanf(&command[1],
               "%d %d  %d %d  %d %d  %d",
               &x_count,
               &y_count,
               &x_offset,
               &y_offset,
               &x_stride,
               &y_stride,
               &render_request_id);
        break;
      case 's':
        sscanf(&command[1], "%d %d %d %d", &w, &h, &padded_w, &padded_h);
        path_tracer.HandleResize(w, h, padded_w, padded_h);
        break;
      case 'k': // <- keyboard input
        break;
      case 'm': // <- mouse movement input
        break;
      case 'b': // <- mouse button input
        break;
    }

    if (command[0] != 'r')
      continue;

    std::vector<unsigned char> buffer = path_tracer.Render(
      x_count, y_count, x_offset, y_offset, x_stride, y_stride);

    printf("rgb buffer %d %d %d\n", x_count, y_count, render_request_id);

    fwrite(&buffer[0], 1, buffer.size(), stdout);
  }

  return EXIT_SUCCESS;
}
