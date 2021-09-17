#pragma once

#include <vision/expr.hpp>
#include <vision/stmt.hpp>

#include <memory>
#include <vector>

namespace vision {

class Lexer;

class Parser final
{
public:
  Parser(Lexer& lexer)
    : m_lexer(lexer)
  {}

  auto ParseStmt() -> std::unique_ptr<Stmt>;

  /// @note Only exposed for testing, not meant to be called directly.
  auto ParseExpr() -> std::unique_ptr<Expr>;

private:
  auto ParseExprList(char l_sym, char r_sym)
    -> std::vector<std::unique_ptr<Expr>>;

  auto ParseAssignStmt() -> std::unique_ptr<Stmt>;

  auto ParseExprStmt() -> std::unique_ptr<Stmt>;

  auto ParseEmptyStmt() -> std::unique_ptr<Stmt>;

  auto ParseIntExpr() -> std::unique_ptr<Expr>;

  auto ParseCallExpr() -> std::unique_ptr<Expr>;

  auto ParseTypeConstructor() -> std::unique_ptr<Expr>;

private:
  Lexer& m_lexer;
};

} // namespace vision
