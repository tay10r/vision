#include <vision/lexer.hpp>

namespace vision {

namespace {

bool
InRange(char c, char lo, char hi)
{
  return (c >= lo) && (c <= hi);
}

bool
IsDigit(char c)
{
  return InRange(c, '0', '9');
}

bool
IsAlpha(char c)
{
  return InRange(c, 'a', 'z') || InRange(c, 'A', 'Z');
}

bool
IsNonDigit(char c)
{
  return IsAlpha(c) || (c == '_');
}

} // namespace

SafeInput::SafeInput(size_t max_buffer_size)
  : m_max_buffer_size(max_buffer_size)
{
  m_buffer.reserve(max_buffer_size);
}

void
SafeInput::Append(const char* data, size_t length)
{
  length =
    std::min(m_max_buffer_size, m_buffer.size() + length) - m_buffer.size();

  size_t original_size = m_buffer.size();

  m_buffer.resize(original_size + length);

  for (size_t i = original_size; i < m_buffer.size(); i++)
    m_buffer[i] = data[i - original_size];
}

void
SafeInput::PopChars(size_t count)
{
  count = std::min(count, m_buffer.size());

  m_buffer.erase(m_buffer.begin(), m_buffer.begin() + count);
}

Lexer::Lexer()
{
  m_type_map["int"] = TypeID::Int;

  m_type_map["float"] = TypeID::Float;

  m_type_map["vec2"] = TypeID::Vec2;
  m_type_map["vec3"] = TypeID::Vec3;
  m_type_map["vec4"] = TypeID::Vec4;

  m_type_map["vec2i"] = TypeID::Vec2i;
  m_type_map["vec3i"] = TypeID::Vec3i;
  m_type_map["vec4i"] = TypeID::Vec4i;
}

std::string
Lexer::ScanIdentifier()
{
  auto identifier = PeekIdentifier();

  if (m_type_map.find(identifier) != m_type_map.end())
    return std::string();

  m_input.PopChars(identifier.size());

  return identifier;
}

const TypeID*
Lexer::ScanType()
{
  auto identifier = PeekIdentifier();

  auto it = m_type_map.find(identifier);
  if (it == m_type_map.end())
    return nullptr;

  m_input.PopChars(identifier.size());

  return &it->second;
}

std::string
Lexer::ScanIdentifier(const std::string& expected)
{
  auto identifier = PeekIdentifier();

  if (identifier == expected) {
    m_input.PopChars(identifier.size());
    return identifier;
  } else {
    return std::string();
  }
}

std::string
Lexer::ScanSpace()
{
  size_t length = 0;

  while (length < Remaining()) {
    char c = m_input[length];
    if ((c == ' ') || (c == '\t'))
      length++;
    else
      break;
  }

  return Produce(length);
}

std::string
Lexer::ScanNewline()
{
  if (m_input[0] == '\n')
    return Produce(1);

  if ((m_input[0] == '\r') && (m_input[1] == '\n'))
    return Produce(2);

  if (m_input[0] == '\r')
    return Produce(1);

  return std::string();
}

std::string
Lexer::ScanSymbol(char c)
{
  if (m_input[0] == c)
    return Produce(1);
  else
    return std::string();
}

std::string
Lexer::ScanInteger()
{
  size_t length = 0;

  if (IsDigit(m_input[0]))
    length = 1;
  else if ((m_input[0] == '-') && IsDigit(m_input[1]))
    length = 2;

  while (length < Remaining()) {
    if (!IsDigit(m_input[length]))
      break;
    else
      length++;
  }

  return Produce(length);
}

std::string
Lexer::PeekIdentifier()
{
  if (!IsNonDigit(m_input[0]))
    return std::string();

  size_t length = 1;

  while (length < Remaining()) {
    char c = m_input[length];
    if (!IsNonDigit(c) && !IsDigit(c))
      break;
    else
      length++;
  }

  std::string identifier;

  identifier.resize(length);

  for (size_t i = 0; i < length; i++)
    identifier[i] = m_input[i];

  return identifier;
}

std::string
Lexer::Produce(size_t length)
{
  std::string output;

  output.resize(length);

  for (size_t i = 0; i < length; i++)
    output[i] = m_input[i];

  m_input.PopChars(length);

  return output;
}

} // namespace vision
