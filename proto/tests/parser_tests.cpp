#include <gtest/gtest.h>

#include <vision/lexer.hpp>
#include <vision/parser.hpp>

#include <vision/assign_stmt.hpp>
#include <vision/empty_stmt.hpp>
#include <vision/expr_stmt.hpp>

#include <vision/call_expr.hpp>
#include <vision/int_expr.hpp>
#include <vision/type_constructor.hpp>

#include <sstream>

using namespace vision;

namespace {

std::string
ParseAndDumpAST(const std::string& input);

} // namespace

TEST(Parser, ParseLetStmt)
{
  auto out = ParseAndDumpAST("let a = 4\n");

  EXPECT_EQ(out, "AssignStmt { VarName: a InitExpr: IntExpr { Value: 4 } }");
}

TEST(Parser, ParseTypeConstructor)
{
  auto out = ParseAndDumpAST("vec4(1 , 2, 3,  4 )\n");

  EXPECT_EQ(out,
            "ExprStmt { TypeConstructor { TypeID: vec4 Args: { IntExpr { "
            "Value: 1 } IntExpr { "
            "Value: 2 } IntExpr { Value: 3 } IntExpr { Value: 4 } } } }");
}

TEST(Parser, ParseIntExpr)
{
  auto out = ParseAndDumpAST("1234\n");

  EXPECT_EQ(out, "ExprStmt { IntExpr { Value: 1234 } }");
}

TEST(Parser, ParseIntExpr2)
{
  auto out = ParseAndDumpAST("-01234\n");

  EXPECT_EQ(out, "ExprStmt { IntExpr { Value: -1234 } }");
}

TEST(Parser, ParseCallExpr)
{
  auto out = ParseAndDumpAST("render()\n");

  EXPECT_EQ(out, "ExprStmt { CallExpr { FuncName: render ArgList: { } } }");
}

TEST(Parser, ParseCallExpr2)
{
  auto out = ParseAndDumpAST("render(4, 5)\n");

  EXPECT_EQ(out,
            "ExprStmt { CallExpr { FuncName: render ArgList: { IntExpr { "
            "Value: 4 } IntExpr { Value: 5 } } } }");
}

TEST(Parser, ParseEmptyStmt)
{
  auto out = ParseAndDumpAST("\n");

  EXPECT_EQ(out, "EmptyStmt {}");
}

namespace {

class ExprPrinter final : public ExprVisitor
{
public:
  ExprPrinter(std::ostream& out)
    : m_out_stream(out)
  {}

  void Visit(const CallExpr& call_expr) override
  {
    m_out_stream << "CallExpr { ";

    m_out_stream << "FuncName: " << call_expr.GetFuncName();

    m_out_stream << ' ';

    m_out_stream << "ArgList: { ";

    for (const auto& arg : call_expr.GetArgs()) {

      ExprPrinter arg_printer(m_out_stream);

      arg->Accept(arg_printer);

      m_out_stream << ' ';
    }

    m_out_stream << "}";

    m_out_stream << " }";
  }

  void Visit(const IntExpr& int_expr) override
  {
    m_out_stream << "IntExpr { Value: " << int_expr.GetValue() << " }";
  }

  void Visit(const TypeConstructor& type_constructor) override
  {
    m_out_stream << "TypeConstructor { ";

    m_out_stream << "TypeID: " << ToString(type_constructor.GetTypeID());

    m_out_stream << ' ';

    m_out_stream << "Args: { ";

    for (const auto& arg : type_constructor.GetArgs()) {

      ExprPrinter printer(m_out_stream);

      arg->Accept(printer);

      m_out_stream << ' ';
    }

    m_out_stream << "}";

    m_out_stream << " }";
  }

private:
  std::ostream& m_out_stream;
};

class StmtPrinter final : public StmtVisitor
{
public:
  StmtPrinter(std::ostream& out_stream)
    : m_out_stream(out_stream)
  {}

  void Visit(const AssignStmt& assign_stmt) override
  {
    m_out_stream << "AssignStmt { ";

    m_out_stream << "VarName: " << assign_stmt.GetVarName();

    m_out_stream << ' ';

    m_out_stream << "InitExpr: ";

    ExprPrinter expr_printer(m_out_stream);

    assign_stmt.GetExpr().Accept(expr_printer);

    m_out_stream << " }";
  }

  void Visit(const ExprStmt& expr_stmt) override
  {
    m_out_stream << "ExprStmt { ";

    ExprPrinter expr_printer(m_out_stream);

    expr_stmt.GetExpr().Accept(expr_printer);

    m_out_stream << " }";
  }

  void Visit(const EmptyStmt& empty_stmt) override
  {
    m_out_stream << "EmptyStmt {}";
  }

private:
  std::ostream& m_out_stream;
};

std::string
ParseAndDumpAST(const std::string& input)
{
  Lexer lexer;

  lexer.Append(input);

  Parser parser(lexer);

  auto stmt = parser.ParseStmt();

  if (!stmt)
    return "<null>";

  std::ostringstream tmp_stream;

  StmtPrinter printer(tmp_stream);

  stmt->Accept(printer);

  return tmp_stream.str();
}

} // namespace
