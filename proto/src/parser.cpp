#include <vision/parser.hpp>

#include <vision/lexer.hpp>

#include <vision/assign_stmt.hpp>
#include <vision/empty_stmt.hpp>
#include <vision/expr_stmt.hpp>

#include <vision/call_expr.hpp>
#include <vision/int_expr.hpp>
#include <vision/type_constructor.hpp>

#include <vision/exception.hpp>

namespace vision {

namespace {

void
ThrowSyntaxError(const Token& token, const std::string_view& msg)
{
  throw SyntaxError(token.line, token.column, msg);
}

} // namespace

Parser::Parser(Lexer& lexer)
{
  m_type_map["int"] = TypeID::Int;

  m_type_map["float"] = TypeID::Float;

  m_type_map["vec2"] = TypeID::Vec2;
  m_type_map["vec3"] = TypeID::Vec3;
  m_type_map["vec4"] = TypeID::Vec4;

  m_type_map["vec2i"] = TypeID::Vec2i;
  m_type_map["vec3i"] = TypeID::Vec3i;
  m_type_map["vec4i"] = TypeID::Vec4i;

  while (!lexer.AtEnd()) {

    auto token = lexer.Scan();

    if (!token.has_value() || token == TokenKind::Space)
      continue;

    m_tokens.emplace_back(std::move(*token));
  }
}

auto
Parser::GetTypeID(const std::optional<Token>& token) const
  -> std::optional<TypeID>
{
  if (token != TokenKind::ID)
    return std::nullopt;

  auto it = m_type_map.find(token->data);
  if (it == m_type_map.end())
    return std::nullopt;

  return it->second;
}

auto
Parser::Remaining() const noexcept -> size_t
{
  if (m_token_offset > m_tokens.size())
    return 0;
  else
    return m_tokens.size() - m_token_offset;
}

auto
Parser::ParseStmt() -> std::unique_ptr<Stmt>
{
  auto assign_stmt = ParseAssignStmt();
  if (assign_stmt)
    return assign_stmt;

  auto expr_stmt = ParseExprStmt();
  if (expr_stmt)
    return expr_stmt;

  auto empty_stmt = ParseEmptyStmt();
  if (empty_stmt)
    return empty_stmt;

  return nullptr;
}

auto
Parser::ParseAssignStmt() -> std::unique_ptr<Stmt>
{
  auto let = Peek(0);
  if (let != "let")
    return nullptr;

  auto var_name = Peek(1);

  if (var_name != TokenKind::ID)
    ThrowSyntaxError(*let, "Expected variable name after this.");

  auto equal_sign = Peek(2);
  if (equal_sign != "=")
    ThrowSyntaxError(*var_name, "Expected a \"=\" after this.");

  Advance(3);

  auto init_expr = ParseExpr();
  if (!init_expr)
    ThrowSyntaxError(*equal_sign, "Expected an expression after this.");

  if (Peek(0) != TokenKind::Newline) {
    ThrowSyntaxError(m_tokens.at(m_token_offset - 1),
                     "Expected newline after this.");
  }

  Advance(1);

  return std::unique_ptr<Stmt>(
    new AssignStmt(std::move(*var_name), std::move(init_expr)));
}

auto
Parser::ParseExprStmt() -> std::unique_ptr<Stmt>
{
  auto expr = ParseExpr();
  if (!expr)
    return nullptr;

  auto newline = Peek(0);

  if (!newline) {
    if (Remaining() > 0)
      ThrowSyntaxError(m_tokens.at(m_token_offset - 1),
                       "Expected newline after this.");
  }

  Advance(1);

  return std::unique_ptr<Stmt>(new ExprStmt(std::move(expr)));
}

auto
Parser::ParseEmptyStmt() -> std::unique_ptr<Stmt>
{
  auto newline_token = Peek(0);

  if (!newline_token)
    return nullptr;

  Advance(1);

  return std::unique_ptr<Stmt>(new EmptyStmt());
}

auto
Parser::ParseExpr() -> std::unique_ptr<Expr>
{
  auto int_expr = ParseIntExpr();
  if (int_expr)
    return int_expr;

  auto call_expr = ParseCallExpr();
  if (call_expr)
    return call_expr;

  auto type_ctor = ParseTypeConstructor();
  if (type_ctor)
    return type_ctor;

  return nullptr;
}

auto
Parser::ParseIntExpr() -> std::unique_ptr<Expr>
{
  std::optional<Token> int_token = Peek(0);
  if (int_token != TokenKind::Int)
    return nullptr;

  Advance(1);

  std::string data_copy(int_token->data);

  return std::unique_ptr<Expr>(new IntExpr(std::stoi(&data_copy[0])));
}

auto
Parser::ParseExprList(char l_sym, char r_sym)
  -> std::optional<std::vector<std::unique_ptr<Expr>>>
{
  std::optional<Token> l_paren = Peek(0);

  if (l_paren != l_sym)
    return std::nullopt;

  Advance(1);

  std::vector<std::unique_ptr<Expr>> args;

  while (Remaining() > 0) {

    if (args.size() > 0) {
      if (Peek(0) != ",")
        break;
      else
        Advance(1);
    }

    auto arg = ParseExpr();
    if (!arg)
      break;

    args.emplace_back(std::move(arg));
  }

  std::optional<Token> r_paren = Peek(0);

  if (r_paren != r_sym)
    ThrowSyntaxError(*l_paren, "Missing \")\".");

  Advance(1);

  return args;
}

auto
Parser::ParseCallExpr() -> std::unique_ptr<Expr>
{
  std::optional<Token> func_name = Peek(0);

  if (func_name != TokenKind::ID)
    return nullptr;

  if (GetTypeID(func_name))
    return nullptr;

  Advance(1);

  std::optional<std::vector<std::unique_ptr<Expr>>> args =
    ParseExprList('(', ')');

  if (!args)
    ThrowSyntaxError(*func_name, "Expected arguments after function name.");

  return std::unique_ptr<Expr>(
    new CallExpr(std::move(*func_name), std::move(*args)));
}

auto
Parser::ParseTypeConstructor() -> std::unique_ptr<Expr>
{
  auto type_name = Peek(0);

  if (type_name != TokenKind::ID)
    return nullptr;

  std::optional<TypeID> type_id = GetTypeID(type_name);

  if (!type_id)
    return nullptr;

  Advance(1);

  std::optional<std::vector<std::unique_ptr<Expr>>> args =
    ParseExprList('(', ')');

  if (!args)
    ThrowSyntaxError(*type_name, "Expected arguments after this.");

  return std::unique_ptr<Expr>(new TypeConstructor(*type_id, std::move(*args)));
}

auto
Parser::Peek(size_t offset) const noexcept -> std::optional<Token>
{
  if ((m_token_offset + offset) < m_tokens.size())
    return m_tokens[m_token_offset + offset];
  else
    return std::nullopt;
}

auto
Parser::MakeMemo() const noexcept -> Memo
{
  return Memo{ m_token_offset };
}

void
Parser::Restore(const Memo& memo)
{
  m_token_offset = memo.token_offset;
}

void
Parser::Advance(size_t count)
{
  m_token_offset += count;
}

} // namespace vision
