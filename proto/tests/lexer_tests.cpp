#include <gtest/gtest.h>

#include <vision/lexer.hpp>

using namespace vision;

TEST(Lexer, ScanIdentifier)
{
  Lexer lexer("some_identifier3 ");

  EXPECT_EQ(lexer.Scan(), "some_identifier3");

  EXPECT_EQ(lexer.Remaining(), 1);
}

TEST(Lexer, ScanIdentifier2)
{
  Lexer lexer("_some_identifier ");

  EXPECT_EQ(lexer.Scan(), "_some_identifier");

  EXPECT_EQ(lexer.Remaining(), 1);
}

TEST(Lexer, ScanIdentifier_Reject)
{
  Lexer lexer("3_asdf");

  EXPECT_NE(lexer.Scan(), TokenKind::ID);

  EXPECT_EQ(lexer.Remaining(), 5);
}

TEST(Lexer, ScanSpace)
{
  Lexer lexer(" \ta");

  EXPECT_EQ(lexer.Scan(), " \t");

  EXPECT_EQ(lexer.Remaining(), 1);
}

TEST(Lexer, ScanSpace2)
{
  Lexer lexer("\t bb");

  EXPECT_EQ(lexer.Scan(), "\t ");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanNewline)
{
  Lexer lexer("\n  ");

  EXPECT_EQ(lexer.Scan(), "\n");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanNewline2)
{
  Lexer lexer("\r\n  ");

  EXPECT_EQ(lexer.Scan(), "\r\n");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanNewline3)
{
  Lexer lexer("\r  ");

  EXPECT_EQ(lexer.Scan(), "\r");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanNewline4)
{
  Lexer lexer("\n\r\n  ");

  EXPECT_EQ(lexer.Scan(), "\n");

  EXPECT_EQ(lexer.Remaining(), 4);
}

TEST(Lexer, ScanInteger)
{
  Lexer lexer("1234  ");

  EXPECT_EQ(lexer.Scan(), "1234");

  EXPECT_EQ(lexer.Remaining(), 2);
}

TEST(Lexer, ScanInteger2)
{
  Lexer lexer("-1234  \n");

  EXPECT_EQ(lexer.Scan(), "-1234");

  EXPECT_EQ(lexer.Remaining(), 3);
}

TEST(Lexer, ScanInteger_Reject)
{
  Lexer lexer(";1234");

  EXPECT_EQ(lexer.Scan(), TokenKind::Symbol);

  EXPECT_EQ(lexer.Remaining(), 4);
}

TEST(Lexer, ScanInteger_Reject2)
{
  Lexer lexer("- 1234");

  EXPECT_EQ(lexer.Scan(), TokenKind::Symbol);

  EXPECT_EQ(lexer.Remaining(), 5);
}
