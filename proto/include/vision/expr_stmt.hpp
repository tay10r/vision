#pragma once

#include <vision/expr.hpp>
#include <vision/stmt.hpp>
#include <vision/stmt_visitor.hpp>

#include <memory>

namespace vision {

class ExprStmt : public Stmt
{
public:
  ExprStmt(std::unique_ptr<Expr>&& expr)
    : m_expr(std::move(expr))
  {}

  void Accept(StmtVisitor& v) const override { v.Visit(*this); }

  auto GetExpr() const noexcept -> const Expr& { return *m_expr; }

private:
  std::unique_ptr<Expr> m_expr;
};

} // namespace vision
