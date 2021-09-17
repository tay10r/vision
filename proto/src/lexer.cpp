#include <vision/lexer.hpp>

namespace vision {

namespace {

bool
InRange(const std::optional<char>& c, char lo, char hi)
{
  return c.has_value() && (c.value() >= lo) && (c.value() <= hi);
}

bool
IsDigit(const std::optional<char>& c)
{
  return InRange(c, '0', '9');
}

bool
IsAlpha(const std::optional<char>& c)
{
  return InRange(c, 'a', 'z') || InRange(c, 'A', 'Z');
}

bool
IsNonDigit(const std::optional<char>& c)
{
  return IsAlpha(c) || (c == '_');
}

} // namespace

Lexer::Lexer(const std::string_view& input)
  : m_input(input)
{}

auto
Lexer::AtEnd() const noexcept -> bool
{
  return m_input_offset >= m_input.size();
}

auto
Lexer::Remaining() const noexcept -> size_t
{
  if (m_input_offset > m_input.size())
    return 0;
  else
    return m_input.size() - m_input_offset;
}

std::optional<Token>
Lexer::Scan()
{
  if (Remaining() == 0)
    return std::nullopt;

  auto id = ScanIdentifier();
  if (id)
    return id;

  auto space = ScanSpace();
  if (space)
    return space;

  auto newline = ScanNewline();
  if (newline)
    return newline;

  auto int_token = ScanInteger();
  if (int_token)
    return int_token;

  return Produce(TokenKind::Symbol, 1);
}

std::optional<Token>
Lexer::ScanIdentifier()
{
  auto identifier = PeekIdentifier();
  if (!identifier)
    return std::nullopt;

  return Produce(TokenKind::ID, identifier->size());
}

std::optional<Token>
Lexer::ScanSpace()
{
  size_t length = 0;

  while (length < Remaining()) {
    std::optional<char> c = Peek(length);
    if ((c == ' ') || (c == '\t'))
      length++;
    else
      break;
  }

  if (length)
    return Produce(TokenKind::Space, length);
  else
    return std::nullopt;
}

std::optional<Token>
Lexer::ScanNewline()
{
  if (Peek(0) == '\n')
    return Produce(TokenKind::Newline, 1);

  if ((Peek(0) == '\r') && (Peek(1) == '\n'))
    return Produce(TokenKind::Newline, 2);

  if (Peek(0) == '\r')
    return Produce(TokenKind::Newline, 1);

  return std::nullopt;
}

std::optional<Token>
Lexer::ScanInteger()
{
  size_t length = 0;

  if (IsDigit(Peek(0)))
    length = 1;
  else if ((Peek(0) == '-') && IsDigit(Peek(1)))
    length = 2;
  else
    return std::nullopt;

  while (length < Remaining()) {
    if (!IsDigit(Peek(length)))
      break;
    else
      length++;
  }

  return Produce(TokenKind::Int, length);
}

std::optional<std::string_view>
Lexer::PeekIdentifier()
{
  if (!IsNonDigit(Peek(0)))
    return std::nullopt;

  size_t length = 1;

  while (length < Remaining()) {
    std::optional<char> c = Peek(length);
    if (!IsNonDigit(c) && !IsDigit(c))
      break;
    else
      length++;
  }

  return m_input.substr(m_input_offset, length);
}

auto
Lexer::Peek(size_t offset) const noexcept -> std::optional<char>
{
  if ((m_input_offset + offset) < m_input.size())
    return m_input[m_input_offset + offset];
  else
    return std::nullopt;
}

auto
Lexer::Produce(TokenKind kind, size_t length) -> std::optional<Token>
{
  Token token{ kind, m_input.substr(m_input_offset, length), m_line, m_column };

  Advance(length);

  return token;
}

void
Lexer::Advance(size_t count)
{
  count = ((m_input_offset + count) < m_input.size())
            ? count
            : m_input.size() - m_input_offset;

  for (size_t i = 0; i < count; i++) {
    if (Peek(i) == '\n') {
      m_column = 1;
      m_line++;
    } else if ((Peek(i) == '\r') && (Peek(i + 1) == '\n')) {
      m_column = 1;
      m_line++;
      i++;
    } else if (Peek(i) == '\r') {
      m_column = 1;
    } else {
      m_column++;
    }
  }

  m_input_offset += count;
}

} // namespace vision
