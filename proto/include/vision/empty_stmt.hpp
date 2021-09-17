#pragma once

#include <vision/stmt.hpp>
#include <vision/stmt_visitor.hpp>

namespace vision {

class EmptyStmt final : public Stmt
{
public:
  void Accept(StmtVisitor& v) const override { v.Visit(*this); }
};

} // namespace vision
