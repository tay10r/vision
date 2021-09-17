#include <vision/parser.hpp>

#include <vision/lexer.hpp>

#include <vision/assign_stmt.hpp>
#include <vision/empty_stmt.hpp>
#include <vision/expr_stmt.hpp>

#include <vision/call_expr.hpp>
#include <vision/int_expr.hpp>
#include <vision/type_constructor.hpp>

#include <stdexcept>
#include <vector>

namespace vision {

namespace {

class SyntaxError final : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

} // namespace

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
  std::string let_token = m_lexer.ScanIdentifier("let");
  if (let_token.empty())
    return nullptr;

  m_lexer.ScanSpace();

  std::string var_name = m_lexer.ScanIdentifier();
  if (var_name.empty())
    return nullptr;

  m_lexer.ScanSpace();

  std::string equal_sign = m_lexer.ScanSymbol('=');
  if (equal_sign.empty())
    return nullptr;

  m_lexer.ScanSpace();

  auto init_expr = ParseExpr();
  if (!init_expr)
    return nullptr;

  return std::unique_ptr<Stmt>(
    new AssignStmt(std::move(var_name), std::move(init_expr)));
}

auto
Parser::ParseExprStmt() -> std::unique_ptr<Stmt>
{
  auto expr = ParseExpr();
  if (!expr)
    return nullptr;

  m_lexer.ScanSpace();

  auto newline = m_lexer.ScanNewline();
  if (newline.empty())
    return nullptr;

  return std::unique_ptr<Stmt>(new ExprStmt(std::move(expr)));
}

auto
Parser::ParseEmptyStmt() -> std::unique_ptr<Stmt>
{
  auto newline_token = m_lexer.ScanNewline();
  if (newline_token.empty())
    return nullptr;

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
  std::string int_token = m_lexer.ScanInteger();
  if (int_token.empty())
    return nullptr;

  return std::unique_ptr<Expr>(new IntExpr(std::stoi(int_token)));
}

auto
Parser::ParseExprList(char l_sym, char r_sym)
  -> std::vector<std::unique_ptr<Expr>>
{
  std::vector<std::unique_ptr<Expr>> args;

  std::string l_paren = m_lexer.ScanSymbol(l_sym);
  if (l_paren.empty())
    return args;

  while (m_lexer.Remaining() > 0) {

    m_lexer.ScanSpace();

    if (args.size() > 0) {

      if (m_lexer.ScanSymbol(',').empty())
        break;

      m_lexer.ScanSpace();
    }

    auto arg = ParseExpr();
    if (!arg)
      break;

    args.emplace_back(std::move(arg));
  }

  std::string r_paren = m_lexer.ScanSymbol(r_sym);
  if (r_paren.empty())
    throw SyntaxError("Function call arguments missing ')'.");

  return args;
}

auto
Parser::ParseCallExpr() -> std::unique_ptr<Expr>
{
  std::string func_name = m_lexer.ScanIdentifier();
  if (func_name.empty())
    return nullptr;

  m_lexer.ScanSpace();

  std::vector<std::unique_ptr<Expr>> args = ParseExprList('(', ')');

  return std::unique_ptr<Expr>(
    new CallExpr(std::move(func_name), std::move(args)));
}

auto
Parser::ParseTypeConstructor() -> std::unique_ptr<Expr>
{
  const TypeID* type_id = m_lexer.ScanType();
  if (!type_id)
    return nullptr;

  m_lexer.ScanSpace();

  std::vector<std::unique_ptr<Expr>> args = ParseExprList('(', ')');

  return std::unique_ptr<Expr>(new TypeConstructor(*type_id, std::move(args)));
}

} // namespace vision
