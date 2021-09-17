#pragma once

#include <vision/expr.hpp>
#include <vision/expr_visitor.hpp>
#include <vision/token.hpp>

#include <memory>
#include <vector>

namespace vision {

class CallExpr : public Expr
{
public:
  CallExpr(Token&& func_name, std::vector<std::unique_ptr<Expr>>&& args)
    : m_func_name(std::move(func_name))
    , m_args(std::move(args))
  {}

  void Accept(ExprVisitor& v) const override { v.Visit(*this); }

  auto GetFuncName() const -> Token { return m_func_name; }

  auto GetArgs() const -> std::vector<const Expr*>
  {
    std::vector<const Expr*> args;

    for (const auto& arg : m_args)
      args.emplace_back(arg.get());

    return args;
  }

private:
  Token m_func_name;

  std::vector<std::unique_ptr<Expr>> m_args;
};

} // namespace vision
