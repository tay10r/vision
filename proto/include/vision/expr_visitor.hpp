#pragma once

namespace vision {

class IntExpr;
class CallExpr;
class TypeConstructor;

class ExprVisitor
{
public:
  virtual ~ExprVisitor() = default;

  virtual void Visit(const CallExpr&) = 0;

  virtual void Visit(const IntExpr&) = 0;

  virtual void Visit(const TypeConstructor&) = 0;
};

} // namespace vision
