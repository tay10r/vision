#include <gtest/gtest.h>

#include <vision/lexer.hpp>

using namespace vision;

TEST(Lexer, ScanIdentifier)
{
  Lexer lexer;

  lexer.Append("some_identifier3 ");

  EXPECT_EQ(lexer.ScanIdentifier(), "some_identifier3");

  EXPECT_EQ(lexer.Remaining(), 1);
}

TEST(Lexer, ScanIdentifier2)
{
  Lexer lexer;

  lexer.Append("_some_identifier ");

  EXPECT_EQ(lexer.ScanIdentifier(), "_some_identifier");

  EXPECT_EQ(lexer.Remaining(), 1);
}

TEST(Lexer, ScanIdentifier_Reject)
{
  Lexer lexer;

  lexer.Append("3_asdf");

  EXPECT_EQ(lexer.ScanIdentifier(), "");

  EXPECT_EQ(lexer.Remaining(), 6);
}

TEST(Lexer, ScanSpace)
{
  Lexer lexer;

  lexer.Append(" \ta");

  EXPECT_EQ(lexer.ScanSpace(), " \t");

  EXPECT_EQ(lexer.Remaining(), 1);
}

TEST(Lexer, ScanSpace2)
{
  Lexer lexer;

  lexer.Append("\t bb");

  EXPECT_EQ(lexer.ScanSpace(), "\t ");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanSpaceReject)
{
  Lexer lexer;

  lexer.Append("\n");

  EXPECT_EQ(lexer.ScanSpace(), "");

  EXPECT_EQ(lexer.Remaining(), 1);
}

TEST(Lexer, ScanNewline)
{
  Lexer lexer;

  lexer.Append("\n  ");

  EXPECT_EQ(lexer.ScanNewline(), "\n");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanNewline2)
{
  Lexer lexer;

  lexer.Append("\r\n  ");

  EXPECT_EQ(lexer.ScanNewline(), "\r\n");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanNewline3)
{
  Lexer lexer;

  lexer.Append("\r  ");

  EXPECT_EQ(lexer.ScanNewline(), "\r");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanNewline4)
{
  Lexer lexer;

  lexer.Append("\n\r\n  ");

  EXPECT_EQ(lexer.ScanNewline(), "\n");

  EXPECT_EQ(lexer.Remaining(), 4);
}

TEST(Lexer, ScanNewline_Reject)
{
  Lexer lexer;

  lexer.Append(" \n");

  EXPECT_EQ(lexer.ScanNewline(), "");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanSymbol)
{
  Lexer lexer;

  lexer.Append("( ");

  EXPECT_EQ(lexer.ScanSymbol('('), "(");

  EXPECT_EQ(lexer.Remaining(), 1);
}

TEST(Lexer, ScanSymbol2)
{
  Lexer lexer;

  lexer.Append("= ");

  EXPECT_EQ(lexer.ScanSymbol('='), "=");

  EXPECT_EQ(lexer.Remaining(), 1);
}

TEST(Lexer, ScanSymbol_Reject)
{
  Lexer lexer;

  lexer.Append("= ");

  EXPECT_EQ(lexer.ScanSymbol('('), "");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanInteger)
{
  Lexer lexer;

  lexer.Append("1234  ");

  EXPECT_EQ(lexer.ScanInteger(), "1234");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanInteger2)
{
  Lexer lexer;

  lexer.Append("-1234  \n");

  EXPECT_EQ(lexer.ScanInteger(), "-1234");

  EXPECT_EQ(lexer.Remaining(), 3);
}

TEST(Lexer, ScanInteger_Reject)
{
  Lexer lexer;

  lexer.Append(";1234");

  EXPECT_EQ(lexer.ScanInteger(), "");

  EXPECT_EQ(lexer.Remaining(), 5);
}

TEST(Lexer, ScanInteger_Reject2)
{
  Lexer lexer;

  lexer.Append("- 1234");

  EXPECT_EQ(lexer.ScanInteger(), "");

  EXPECT_EQ(lexer.Remaining(), 6);
}
