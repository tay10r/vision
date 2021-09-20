#include <gtest/gtest.h>

#include <sstream>

#include "schedule.hpp"
#include "vertex.hpp"

using namespace vision::gui;

TEST(Schedule, GetPartitionCount)
{
  Schedule schedule(32, 32, 2);

  EXPECT_EQ(schedule.GetPartitionCount(), 16);
}

TEST(Schedule, GetPreviewCount)
{
  Schedule schedule(32, 32, 2);

  EXPECT_EQ(schedule.GetPreviewCount(), 2);
}

TEST(Schedule, GetTextureSize)
{
  Schedule schedule(5, 6, 0);
  EXPECT_EQ(schedule.GetTextureWidth(), 5);
  EXPECT_EQ(schedule.GetTextureHeight(), 6);
}

TEST(Schedule, GetTextureSize2)
{
  Schedule schedule(5, 6, 1);
  EXPECT_EQ(schedule.GetTextureWidth(), 6);
  EXPECT_EQ(schedule.GetTextureHeight(), 6);
}

TEST(Schedule, GetTextureSize3)
{
  Schedule schedule(5, 7, 1);
  EXPECT_EQ(schedule.GetTextureWidth(), 6);
  EXPECT_EQ(schedule.GetTextureHeight(), 8);
}

TEST(Schedule, GetTextureSize4)
{
  Schedule schedule(5, 7, 2);
  EXPECT_EQ(schedule.GetTextureWidth(), 8);
  EXPECT_EQ(schedule.GetTextureHeight(), 8);
}

TEST(Schedule, GetTextureSize5)
{
  Schedule schedule(5, 7, 3);
  EXPECT_EQ(schedule.GetTextureWidth(), 8);
  EXPECT_EQ(schedule.GetTextureHeight(), 8);
}

TEST(Schedule, GetTextureSize6)
{
  Schedule schedule(5, 7, 4);
  EXPECT_EQ(schedule.GetTextureWidth(), 16);
  EXPECT_EQ(schedule.GetTextureHeight(), 16);
}

TEST(Schedule, GetVertexBuffer)
{
  static_assert(sizeof(Vertex) == (sizeof(float) * 2));

  Schedule schedule(3, 5, 0);

  std::vector<Vertex> vertices = schedule.GetVertexBuffer();

  const Vertex p00(0.0f, 0.0f);
  const Vertex p10(1.0f / 3.0f, 0.0f);
  const Vertex p01(0.0f, 1.0f / 5.0f);
  const Vertex p11(1.0f / 3.0f, 1.0f / 5.0f);

  EXPECT_EQ(vertices.size(), 90);

  EXPECT_EQ(vertices[0], p00);
  EXPECT_EQ(vertices[1], p01);
  EXPECT_EQ(vertices[2], p10);

  EXPECT_EQ(vertices[3], p01);
  EXPECT_EQ(vertices[4], p11);
  EXPECT_EQ(vertices[5], p10);
}

TEST(Schedule, GetStride)
{
  Schedule schedule(3, 5, 0);
  EXPECT_EQ(schedule.GetVerticalStride(), 1);
  EXPECT_EQ(schedule.GetHorizontalStride(), 1);
}

TEST(Schedule, GetStride2)
{
  Schedule schedule(3, 5, 1);
  EXPECT_EQ(schedule.GetVerticalStride(), 2);
  EXPECT_EQ(schedule.GetHorizontalStride(), 2);
}

TEST(Schedule, GetStride3)
{
  Schedule schedule(11, 13, 2);
  EXPECT_EQ(schedule.GetVerticalStride(), 4);
  EXPECT_EQ(schedule.GetHorizontalStride(), 4);
}

namespace {

std::string
PrintRequests(Schedule& schedule)
{
  std::ostringstream stream;

  while (schedule.GetRemainingRenderRequests() > 0) {

    RenderRequest req = schedule.GetRenderRequest();

    stream << "offset = (";
    stream << req.x_pixel_offset;
    stream << ", ";
    stream << req.y_pixel_offset;
    stream << ')';

    stream << "; ";

    stream << "stride = (";
    stream << req.x_pixel_stride;
    stream << ", ";
    stream << req.y_pixel_stride;
    stream << ')';

    stream << "; ";

    stream << "count = (";
    stream << req.x_pixel_count;
    stream << ", ";
    stream << req.y_pixel_count;
    stream << ')';

    stream << '\n';

    schedule.NextRenderRequest();
  }

  return stream.str();
}

} // namespace

TEST(Schedule, PopRequest)
{
  Schedule schedule(5, 7, 0);

  std::string out = PrintRequests(schedule);

  EXPECT_EQ(out, "offset = (0, 0); stride = (1, 1); count = (5, 7)\n");
}

TEST(Schedule, PopRequest2)
{
  Schedule schedule(5, 7, 1);

  std::string out = PrintRequests(schedule);

  EXPECT_EQ(out,
            "offset = (0, 0); stride = (2, 2); count = (2, 3)\n"
            "offset = (1, 0); stride = (2, 2); count = (2, 3)\n"
            "offset = (0, 1); stride = (2, 2); count = (2, 3)\n"
            "offset = (1, 1); stride = (2, 2); count = (2, 3)\n");
}

TEST(Schedule, PopRequest3)
{
  Schedule schedule(16, 8, 2);

  std::string out = PrintRequests(schedule);

  EXPECT_EQ(out,
            "offset = (0, 0); stride = (4, 4); count = (3, 1)\n"
            "offset = (2, 0); stride = (4, 4); count = (3, 1)\n"
            "offset = (0, 2); stride = (4, 4); count = (3, 1)\n"
            "offset = (2, 2); stride = (4, 4); count = (3, 1)\n"
            "offset = (1, 0); stride = (4, 4); count = (3, 1)\n"
            "offset = (0, 1); stride = (4, 4); count = (3, 1)\n"
            "offset = (1, 1); stride = (4, 4); count = (3, 1)\n"
            "offset = (3, 0); stride = (4, 4); count = (3, 1)\n"
            "offset = (2, 1); stride = (4, 4); count = (3, 1)\n"
            "offset = (3, 1); stride = (4, 4); count = (3, 1)\n"
            "offset = (1, 2); stride = (4, 4); count = (3, 1)\n"
            "offset = (0, 3); stride = (4, 4); count = (3, 1)\n"
            "offset = (1, 3); stride = (4, 4); count = (3, 1)\n"
            "offset = (3, 2); stride = (4, 4); count = (3, 1)\n"
            "offset = (2, 3); stride = (4, 4); count = (3, 1)\n"
            "offset = (3, 3); stride = (4, 4); count = (3, 1)\n");
}

namespace {

std::string
Print(const std::vector<PreviewOperation>& ops)
{
  std::ostringstream stream;

  for (const auto& op : ops) {

    stream << "offset = (";
    stream << op.x_pixel_offset;
    stream << ", ";
    stream << op.y_pixel_offset;
    stream << ')';

    stream << "; ";

    stream << "stride = (";
    stream << op.x_pixel_stride;
    stream << ", ";
    stream << op.y_pixel_stride;
    stream << ')';

    stream << '\n';
  }

  return stream.str();
}
} // namespace

TEST(Schedule, GetPreviewOperations)
{
  Schedule schedule(16, 8, 2);

  auto ops = schedule.GetPreviewOperations();

  EXPECT_EQ(ops.empty(), true);
}

TEST(Schedule, GetPreviewOperations2)
{
  Schedule schedule(16, 8, 2);

  schedule.NextRenderRequest();

  std::string out = Print(schedule.GetPreviewOperations());

  EXPECT_EQ(out, "offset = (0, 0); stride = (1, 1)\n");
}

TEST(Schedule, GetPreviewOperations3)
{
  Schedule schedule(16, 8, 2);

  schedule.NextRenderRequest();
  schedule.NextRenderRequest();

  std::string out = Print(schedule.GetPreviewOperations());

  EXPECT_EQ(out, "offset = (0, 0); stride = (1, 1)\n");
}

TEST(Schedule, GetPreviewOperations4)
{
  Schedule schedule(16, 8, 2);

  schedule.NextRenderRequest();
  schedule.NextRenderRequest();
  schedule.NextRenderRequest();
  schedule.NextRenderRequest();

  std::string out = Print(schedule.GetPreviewOperations());

  EXPECT_EQ(out,
            "offset = (0, 0); stride = (2, 2)\n"
            "offset = (1, 0); stride = (2, 2)\n"
            "offset = (0, 1); stride = (2, 2)\n"
            "offset = (1, 1); stride = (2, 2)\n");
}

TEST(Schedule, GetPreviewOperations5)
{
  Schedule schedule(16, 8, 2);

  schedule.NextRenderRequest();
  schedule.NextRenderRequest();
  schedule.NextRenderRequest();
  schedule.NextRenderRequest();
  schedule.NextRenderRequest();

  std::string out = Print(schedule.GetPreviewOperations());

  EXPECT_EQ(out,
            "offset = (0, 0); stride = (2, 2)\n"
            "offset = (1, 0); stride = (2, 2)\n"
            "offset = (0, 1); stride = (2, 2)\n"
            "offset = (1, 1); stride = (2, 2)\n");
}

TEST(Schedule, GetPreviewOperations6)
{
  Schedule schedule(16, 8, 2);

  for (int i = 0; i < 16; i++)
    schedule.NextRenderRequest();

  std::string out = Print(schedule.GetPreviewOperations());

  EXPECT_EQ(out,
            "offset = (0, 0); stride = (4, 4)\n"
            "offset = (2, 0); stride = (4, 4)\n"
            "offset = (0, 2); stride = (4, 4)\n"
            "offset = (2, 2); stride = (4, 4)\n"
            "offset = (1, 0); stride = (4, 4)\n"
            "offset = (0, 1); stride = (4, 4)\n"
            "offset = (1, 1); stride = (4, 4)\n"
            "offset = (3, 0); stride = (4, 4)\n"
            "offset = (2, 1); stride = (4, 4)\n"
            "offset = (3, 1); stride = (4, 4)\n"
            "offset = (1, 2); stride = (4, 4)\n"
            "offset = (0, 3); stride = (4, 4)\n"
            "offset = (1, 3); stride = (4, 4)\n"
            "offset = (3, 2); stride = (4, 4)\n"
            "offset = (2, 3); stride = (4, 4)\n"
            "offset = (3, 3); stride = (4, 4)\n");
}
