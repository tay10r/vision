#pragma once

#include <vision/expr.hpp>
#include <vision/expr_visitor.hpp>

namespace vision {

class IntExpr final : public Expr
{
public:
  IntExpr(int value)
    : m_value(value)
  {}

  void Accept(ExprVisitor& v) const override { v.Visit(*this); }

  auto GetValue() const noexcept { return m_value; }

private:
  int m_value = 0;
};

} // namespace vision
