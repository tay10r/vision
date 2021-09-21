#include "response.hpp"

#include "lexer.hpp"

#include <optional>
#include <string>
#include <vector>

#include <stddef.h>

namespace vision::gui {

namespace {

struct TokenBuffer final
{
public:
  void Append(const Token& tok) { m_tokens.emplace_back(tok); }

  bool Empty() const noexcept { return m_tokens.empty(); }

  size_t Size() const noexcept { return m_tokens.size(); }

  auto operator[](size_t index) const -> std::optional<Token>
  {
    if (index >= m_tokens.size())
      return std::nullopt;
    else
      return m_tokens[index];
  }

private:
  std::vector<Token> m_tokens;
};

class ResponseParserImpl final : public ResponseParser
{
public:
  ResponseParserImpl(ResponseObserver& o)
    : m_observer(o)
  {}

  bool Write(const char* data, size_t length) override
  {
    if ((m_buffer.size() + length) > m_buffer_max) {
      m_observer.OnBufferOverflow();
      m_buffer.clear();
      return false;
    }

    size_t old_size = m_buffer.size();

    m_buffer.resize(old_size + length);

    for (size_t i = 0; i < length; i++)
      m_buffer[old_size + i] = data[i];

    ParseBuffer();

    return true;
  }

  void SetMaxBufferSize(size_t max_size) override { m_buffer_max = max_size; }

private:
  bool ParseBuffer()
  {
    std::string line = GetLine();

    if (line.empty())
      return false;

    TokenBuffer tokens = GetLineTokens(line);

    if (tokens.Empty())
      return false;

    if (ParseRGBBuffer(line, tokens)) {
      return true;
    } else {
      HandleInvalidInput("Header line is not recognizable.");
      return false;
    }
  }

  bool ParseRGBBuffer(const std::string& line, const TokenBuffer& tokens)
  {
    if (tokens[0] != "rgb")
      return false;

    if (tokens[1] != "buffer")
      return false;

    if (tokens.Size() < 3) {
      HandleInvalidInput("Width and height are missing.");
      return true;
    } else if (tokens.Size() < 4) {
      HandleInvalidInput("Height is missing.");
      return true;
    }

    if (tokens[2] != TokenKind::Int) {
      HandleInvalidInput("Width is not an integer.");
    } else if (tokens[3] != TokenKind::Int) {
      HandleInvalidInput("Height is not an integer.");
      return true;
    }

    if (tokens.Size() != 4) {
      HandleInvalidInput("Trailing tokens after width and height.");
      return true;
    }

    const std::string w_str(std::string(tokens[2]->data));
    const std::string h_str(std::string(tokens[3]->data));

    const int w = std::atoi(w_str.c_str());
    const int h = std::atoi(h_str.c_str());

    if (w < 0) {
      HandleInvalidInput("Width is negative.");
      return true;
    } else if (h < 0) {
      HandleInvalidInput("Height is negative.");
      return true;
    }

    size_t rgb_buffer_size = w * h * 3;

    if (line.size() > m_buffer.size()) {
      // This is not really likely, just a safety check.
      return true;
    }

    if ((m_buffer.size() - line.size()) < rgb_buffer_size)
      return true;

    std::vector<char> rgb_buf(m_buffer.begin() + line.size(), m_buffer.end());

    const unsigned char* rgb_ptr = (const unsigned char*)rgb_buf.data();

    m_observer.OnRGBBuffer(rgb_ptr, size_t(w), size_t(h));

    Advance(line.size(), rgb_buffer_size);

    return true;
  }

  void EraseBufferPrefix(size_t size)
  {
    size = std::min(size, m_buffer.size());

    m_buffer.erase(m_buffer.begin(), m_buffer.begin() + size);
  }

  void HandleInvalidInput(const std::string_view& reason)
  {
    m_buffer.clear();

    m_observer.OnInvalidResponse(reason);
  }

  TokenBuffer GetLineTokens(const std::string_view& line)
  {
    Lexer lexer(line);

    TokenBuffer tokens;

    while (!lexer.AtEnd()) {
      const std::optional<Token> token = lexer.Scan();
      if (!token)
        break;
      else if (token == TokenKind::Space)
        continue;
      else if (token == TokenKind::Newline)
        break;

      tokens.Append(token.value());
    }

    return tokens;
  }

  std::string GetLine() const
  {
    size_t length = 0;

    std::string line;

    while (!OutOfBounds(length)) {
      line.push_back(Peek(length));

      if (line.back() == '\n')
        return line;
      else if ((line.size() > 1) && (line[line.size() - 2] == '\r') &&
               (line[line.size() - 1] == '\n'))
        return line;
      else
        length++;
    }

    return std::string();
  }

  char Peek(size_t offset) const noexcept
  {
    if (offset >= m_buffer.size())
      return 0;
    else
      return m_buffer[offset];
  }

  bool OutOfBounds(size_t index) const noexcept
  {
    return index >= m_buffer.size();
  }

  void Advance(size_t line_size, size_t data_size)
  {
    const size_t total_size = line_size + data_size;

    const size_t clipped_size = std::min(total_size, m_buffer.size());

    m_buffer.erase(m_buffer.begin(), m_buffer.begin() + clipped_size);
  }

private:
  ResponseObserver& m_observer;

  /// Beyond this response size is considered to be invalid. This is the size of
  /// a 4096 by 4096 RGB buffer (24-bit) with the header `rgb 4096 4096\r\n`.
  size_t m_buffer_max = 50331648 + 15;

  std::vector<char> m_buffer;
};

} // namespace

auto
ResponseParser::Create(ResponseObserver& observer)
  -> std::unique_ptr<ResponseParser>
{
  return std::unique_ptr<ResponseParser>(new ResponseParserImpl(observer));
}

} // namespace vision::gui
