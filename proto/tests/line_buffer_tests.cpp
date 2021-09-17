#include <gtest/gtest.h>

#include <vision/line_buffer.hpp>

using namespace vision;

TEST(LineBuffer, Empty)
{
  LineBuffer line_buffer;

  EXPECT_EQ(line_buffer.Write("", 0), 0);

  EXPECT_EQ(line_buffer.IsTerminated(), false);

  EXPECT_EQ(line_buffer.GetAvailableLineCount(), 0);
}

TEST(LineBuffer, OneChar)
{
  LineBuffer line_buffer;

  EXPECT_EQ(line_buffer.Write("a", 1), 1);

  EXPECT_EQ(line_buffer.IsTerminated(), false);

  EXPECT_EQ(line_buffer.GetAvailableLineCount(), 0);
}

TEST(LineBuffer, OneLine)
{
  LineBuffer line_buffer;

  EXPECT_EQ(line_buffer.Write("a\n", 2), 2);

  EXPECT_EQ(line_buffer.GetAvailableLineCount(), 1);

  EXPECT_EQ(line_buffer.IsTerminated(), false);

  EXPECT_EQ(line_buffer.PopLine(), "a\n");

  EXPECT_EQ(line_buffer.GetAvailableLineCount(), 0);

  EXPECT_EQ(line_buffer.PopLine(), "");
}

TEST(LineBuffer, TerminatingLine)
{
  LineBuffer line_buffer;

  EXPECT_EQ(line_buffer.Write("\n\x01", 2), 1);

  EXPECT_EQ(line_buffer.IsTerminated(), true);
}
