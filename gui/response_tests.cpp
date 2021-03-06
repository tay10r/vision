#include <gtest/gtest.h>

#include "response.hpp"

#include <sstream>

using namespace vision::gui;

#define BINARY_STRING(str) std::string(str, sizeof(str))

namespace {

class ResponseLogger final : public ResponseObserver
{
public:
  ResponseLogger(std::ostream& output)
    : m_output(output)
  {}

  void OnInvalidResponse(const std::string_view& reason) override
  {
    m_output << "InvalidResponse: " << reason << '\n';
  }

  void OnBufferOverflow(size_t) override { m_output << "BufferOverflow\n"; }

  void OnRGBBuffer(const unsigned char*, size_t w, size_t h, size_t id) override
  {
    m_output << "RGBBuffer " << w << ' ' << h << ' ' << id << '\n';
  }

private:
  std::ostream& m_output;
};

void
Write(ResponseParser& parser, const std::string& str)
{
  parser.Write(str.data(), str.size());
}

} // namespace

TEST(Response, ExceedBufferMax)
{
  std::ostringstream stream;

  ResponseLogger logger(stream);

  std::unique_ptr<ResponseParser> parser = ResponseParser::Create(logger);

  parser->SetMaxBufferSize(2);

  parser->Write(" ", 1);

  EXPECT_EQ(stream.str(), "");

  parser->Write(" ", 1);

  EXPECT_EQ(stream.str(), "");

  parser->Write(" ", 1);

  EXPECT_EQ(stream.str(), "BufferOverflow\n");
}

namespace {

std::string
ParseAndLog(const std::string& input);

} // namespace

TEST(Response, RGBBuffer)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer 2 3 0\n"
                                              "\x00\x11\x00"
                                              "\x00\x55\x00"
                                              "\x22\x00\x33"
                                              "\x66\x00\x88"
                                              "\x00\x44\x00"
                                              "\x00\x77\x00"));

  EXPECT_EQ(out, "RGBBuffer 2 3 0\n");
}

TEST(Response, RGBBuffer_NegativeWidth)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer -4 1 0\n"));

  EXPECT_EQ(out, "InvalidResponse: Width is negative.\n");
}

TEST(Response, RGBBuffer_NegativeHeight)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer 4 -1 0\n"));

  EXPECT_EQ(out, "InvalidResponse: Height is negative.\n");
}

TEST(Response, RGBBuffer_NegativeRequestID)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer 4 1 -1\n"));

  EXPECT_EQ(out, "InvalidResponse: Request ID is negative.\n");
}

TEST(Response, RGBBuffer_MissingHeightAndRequestID)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer 4\n"));

  EXPECT_EQ(out, "InvalidResponse: Height and request ID are missing.\n");
}

TEST(Response, RGBBuffer_MissingWidthHeightAndRequestID)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer\n"));

  EXPECT_EQ(out,
            "InvalidResponse: Width, height and request ID are missing.\n");
}

TEST(Response, RGBBuffer_MissingRequestID)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer 0 1\n"));

  EXPECT_EQ(out, "InvalidResponse: Request ID is missing.\n");
}

TEST(Response, RGBBuffer_HeightIsNotInteger)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer 4 ; 0\n"));

  EXPECT_EQ(out, "InvalidResponse: Height is not an integer.\n");
}

TEST(Response, RGBBuffer_WidthIsNotInteger)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer ; 4 0\n"));

  EXPECT_EQ(out, "InvalidResponse: Width is not an integer.\n");
}

TEST(Response, RGBBuffer_RequestIDIsNotInteger)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer 4 1 ?\n"));

  EXPECT_EQ(out, "InvalidResponse: Request ID is not an integer.\n");
}

TEST(Response, RGBBuffer_TrailingTokens)
{
  std::string out = ParseAndLog(BINARY_STRING("rgb buffer 1 4 0 asdf\n"));

  EXPECT_EQ(out, "InvalidResponse: Trailing tokens after request ID.\n");
}

TEST(Response, UnrecognizedHeader)
{
  std::string out = ParseAndLog(BINARY_STRING("bad input\n"));

  EXPECT_EQ(out, "InvalidResponse: Header line is not recognizable.\n");
}

namespace {

std::string
ParseAndLog(const std::string& input)
{
  std::ostringstream stream;

  ResponseLogger logger(stream);

  std::unique_ptr<ResponseParser> parser = ResponseParser::Create(logger);

  parser->Write(&input[0], input.size());

  return stream.str();
}

} // namespace
