#pragma once

namespace vision {

class StmtVisitor;

class Stmt
{
public:
  virtual ~Stmt() = default;

  virtual void Accept(StmtVisitor&) const = 0;
};

} // namespace vision
