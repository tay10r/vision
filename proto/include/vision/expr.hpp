#pragma once

namespace vision {

class ExprVisitor;

class Expr
{
public:
  virtual ~Expr() = default;

  virtual void Accept(ExprVisitor& v) const = 0;
};

} // namespace vision
