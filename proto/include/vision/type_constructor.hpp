#pragma once

#include <vision/expr.hpp>
#include <vision/expr_visitor.hpp>
#include <vision/type_id.hpp>

namespace vision {

class TypeConstructor : public Expr
{
public:
  TypeConstructor(TypeID type_id, std::vector<std::unique_ptr<Expr>>&& args)
    : m_type_id(type_id)
    , m_args(std::move(args))
  {}

  void Accept(ExprVisitor& v) const override { v.Visit(*this); }

  auto GetTypeID() const noexcept { return m_type_id; }

  auto GetArgs() const -> std::vector<const Expr*>
  {
    std::vector<const Expr*> args;

    for (const auto& arg : m_args)
      args.emplace_back(arg.get());

    return args;
  }

private:
  TypeID m_type_id;

  std::vector<std::unique_ptr<Expr>> m_args;
};

} // namespace vision
