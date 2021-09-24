#include "response.hpp"

#include "lexer.hpp"

#include <algorithm>
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
  {
    m_buffer.reserve(m_buffer_max);
  }

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

  void SetMaxBufferSize(size_t max_size) override
  {
    m_buffer_max = max_size;

    m_buffer.reserve(max_size);
  }

  size_t GetMaxBufferSize() const noexcept override { return m_buffer_max; }

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
      HandleInvalidInput("Width, height and request ID are missing.");
      return true;
    } else if (tokens.Size() < 4) {
      HandleInvalidInput("Height and request ID are missing.");
      return true;
    } else if (tokens.Size() < 5) {
      HandleInvalidInput("Request ID is missing.");
      return true;
    }

    if (tokens[2] != TokenKind::Int) {
      HandleInvalidInput("Width is not an integer.");
    } else if (tokens[3] != TokenKind::Int) {
      HandleInvalidInput("Height is not an integer.");
      return true;
    } else if (tokens[4] != TokenKind::Int) {
      HandleInvalidInput("Request ID is not an integer.");
      return true;
    }

    if (tokens.Size() != 5) {
      HandleInvalidInput("Trailing tokens after request ID.");
      return true;
    }

    const std::string w_str(std::string(tokens[2]->data));
    const std::string h_str(std::string(tokens[3]->data));
    const std::string id_str(std::string(tokens[4]->data));

    const int w = std::atoi(w_str.c_str());
    const int h = std::atoi(h_str.c_str());
    const int id = std::atoi(id_str.c_str());

    if (w < 0) {
      HandleInvalidInput("Width is negative.");
      return true;
    } else if (h < 0) {
      HandleInvalidInput("Height is negative.");
      return true;
    } else if (id < 0) {
      HandleInvalidInput("Request ID is negative.");
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

    m_observer.OnRGBBuffer(rgb_ptr, size_t(w), size_t(h), size_t(id));

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
    std::vector<char>::const_iterator it =
      std::find(m_buffer.begin(), m_buffer.end(), '\n');

    if (it == m_buffer.end())
      return std::string();

    size_t length = std::distance(m_buffer.begin(), it) + 1;

    std::string line;

    for (size_t i = 0; i < length; i++)
      line.push_back(m_buffer.at(i));

    return line;
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

  /// Beyond this response size (16 MiB) is considered to be invalid.
  size_t m_buffer_max = 16777216;

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
