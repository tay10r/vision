#pragma once

#include <vision/expr.hpp>
#include <vision/stmt.hpp>
#include <vision/stmt_visitor.hpp>

namespace vision {

class AssignStmt : public Stmt
{
public:
  AssignStmt(std::string&& var_name, std::unique_ptr<Expr>&& expr)
    : m_var_name(std::move(var_name))
    , m_expr(std::move(expr))
  {}

  void Accept(StmtVisitor& v) const override { v.Visit(*this); }

  auto GetVarName() const noexcept -> const std::string& { return m_var_name; }

  auto GetExpr() const noexcept -> const Expr& { return *m_expr; }

private:
  std::string m_var_name;

  std::unique_ptr<Expr> m_expr;
};

} // namespace vision
