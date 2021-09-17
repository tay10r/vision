#pragma once

#include <vision/expr.hpp>
#include <vision/stmt.hpp>
#include <vision/stmt_visitor.hpp>
#include <vision/token.hpp>

namespace vision {

class AssignStmt : public Stmt
{
public:
  AssignStmt(Token&& var_name, std::unique_ptr<Expr>&& expr)
    : m_var_name(std::move(var_name))
    , m_expr(std::move(expr))
  {}

  void Accept(StmtVisitor& v) const override { v.Visit(*this); }

  auto GetVarName() const noexcept -> Token { return m_var_name; }

  auto GetExpr() const noexcept -> const Expr& { return *m_expr; }

private:
  Token m_var_name;

  std::unique_ptr<Expr> m_expr;
};

} // namespace vision
