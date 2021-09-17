#pragma once

namespace vision {

class AssignStmt;
class ExprStmt;
class EmptyStmt;

class StmtVisitor
{
public:
  virtual ~StmtVisitor() = default;

  virtual void Visit(const AssignStmt&) = 0;

  virtual void Visit(const ExprStmt&) = 0;

  virtual void Visit(const EmptyStmt&) = 0;
};

} // namespace vision
