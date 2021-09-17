#include <vision/interpreter.hpp>

#include <vision/assign_stmt.hpp>
#include <vision/empty_stmt.hpp>
#include <vision/expr_stmt.hpp>

#include <vision/call_expr.hpp>
#include <vision/int_expr.hpp>
#include <vision/type_constructor.hpp>

#include <vision/lexer.hpp>
#include <vision/line_buffer.hpp>
#include <vision/parser.hpp>

#include <ostream>
#include <stdexcept>

namespace vision {

namespace {

class InterpError : public std::runtime_error
{
public:
  InterpError(const char* msg)
    : std::runtime_error(msg)
  {}
};

enum class AtomicTypeID
{
  Int,
  Float
};

template<typename Scalar, typename ValueType>
ValueType*
MakeValue(const std::vector<Scalar>&);

template<>
Int*
MakeValue(const std::vector<int>& values)
{
  return new Int(values.at(0));
}

template<>
Float*
MakeValue(const std::vector<float>& values)
{
  return new Float(values.at(0));
}

template<>
Vec2*
MakeValue(const std::vector<float>& values)
{
  return new Vec2(values.at(0), values.at(1));
}

template<>
Vec3*
MakeValue(const std::vector<float>& values)
{
  return new Vec3(values.at(0), values.at(1), values.at(2));
}

template<>
Vec4*
MakeValue(const std::vector<float>& values)
{
  return new Vec4(values.at(0), values.at(1), values.at(2), values.at(3));
}

template<>
Vec2i*
MakeValue(const std::vector<int>& values)
{
  return new Vec2i(values.at(0), values.at(1));
}

template<>
Vec3i*
MakeValue(const std::vector<int>& values)
{
  return new Vec3i(values.at(0), values.at(1), values.at(2));
}

template<>
Vec4i*
MakeValue(const std::vector<int>& values)
{
  return new Vec4i(values.at(0), values.at(1), values.at(2), values.at(3));
}

class ExprInterpreter;

template<typename Scalar>
class TypeInstancer final : public ExprVisitor
{
public:
  TypeInstancer(ExprInterpreter& expr_interpreter, size_t max_elements)
    : m_expr_interpreter(expr_interpreter)
    , m_max_elements(max_elements)
  {}

  auto GetInterpreter() -> Interpreter&;

  auto GetValues() -> std::vector<Scalar> { return std::move(m_values); }

  void Visit(const IntExpr&) override
  {
    throw InterpError("Unexpected integer expression.");
  }

  void Visit(const CallExpr&) override {}

  void Visit(const TypeConstructor&) override
  {
    throw InterpError("Unexpected type constructor");
  }

private:
  void AddValue(Scalar value)
  {
    m_values.emplace_back(value);

    if (m_values.size() > m_max_elements)
      throw InterpError("Too many values given to type constructor.");
  }

private:
  ExprInterpreter& m_expr_interpreter;

  size_t m_max_elements;

  std::vector<Scalar> m_values;
};

template<>
void
TypeInstancer<int>::Visit(const IntExpr& int_expr)
{
  m_values.emplace_back(int_expr.GetValue());
}

template<>
void
TypeInstancer<int>::Visit(const TypeConstructor& type_constructor)
{
  switch (type_constructor.GetTypeID()) {
    case TypeID::Int:
    case TypeID::Vec2i:
    case TypeID::Vec3i:
    case TypeID::Vec4i:
      for (const Expr* arg : type_constructor.GetArgs())
        arg->Accept(*this);
      break;
    default:
      throw InterpError(
        "Only integer types allowed in integer type constructor.");
      break;
  }
}

class StmtInterpreter;

class ExprInterpreter final : public ExprVisitor
{
public:
  ExprInterpreter(StmtInterpreter& stmt_interpreter)
    : m_stmt_interpreter(stmt_interpreter)
  {}

  Interpreter& GetInterpreter();

  auto PopValue() -> std::unique_ptr<Value>
  {
    if (m_stack.empty())
      return nullptr;

    auto top_value = std::move(m_stack.back());

    m_stack.pop_back();

    return top_value;
  }

  void Visit(const CallExpr& call_expr) override
  {
    std::vector<std::unique_ptr<Value>> call_values;

    for (const Expr* arg : call_expr.GetArgs()) {

      ExprInterpreter arg_interpreter(m_stmt_interpreter);

      arg->Accept(arg_interpreter);

      call_values.emplace_back(arg_interpreter.PopValue());
    }

    std::vector<const Value*> call_value_ptrs;

    for (const auto& call_value : call_values)
      call_value_ptrs.emplace_back(call_value.get());

    std::string func_name = call_expr.GetFuncName();

    Function* func = GetInterpreter().FindFunc(func_name);
    if (!func)
      throw InterpError("Failed to find function.");

    m_stack.emplace_back(func->Call(call_value_ptrs));
  }

  void Visit(const IntExpr& int_expr) override
  {
    m_stack.emplace_back(new Int(int_expr.GetValue()));
  }

  void Visit(const TypeConstructor& type_constructor) override
  {
    switch (type_constructor.GetTypeID()) {
      case TypeID::Int:
        InstanceType<int, Int>(type_constructor.GetArgs(), 1);
        break;
      case TypeID::Float:
        InstanceType<float, Float>(type_constructor.GetArgs(), 1);
        break;
      case TypeID::Vec2:
        InstanceType<float, Vec2>(type_constructor.GetArgs(), 2);
        break;
      case TypeID::Vec3:
        InstanceType<float, Vec3>(type_constructor.GetArgs(), 3);
        break;
      case TypeID::Vec4:
        InstanceType<float, Vec4>(type_constructor.GetArgs(), 4);
        break;
      case TypeID::Vec2i:
        InstanceType<int, Vec2i>(type_constructor.GetArgs(), 2);
        break;
      case TypeID::Vec3i:
        InstanceType<int, Vec3i>(type_constructor.GetArgs(), 3);
        break;
      case TypeID::Vec4i:
        InstanceType<int, Vec4i>(type_constructor.GetArgs(), 4);
        break;
    }
  }

private:
  template<typename Scalar, typename ValueType>
  void InstanceType(const std::vector<const Expr*>& args, size_t max_elements)
  {
    TypeInstancer<Scalar> type_instancer(*this, max_elements);

    for (const Expr* arg : args) {
      arg->Accept(type_instancer);
    }

    std::vector<Scalar> values = type_instancer.GetValues();

    m_stack.emplace_back(MakeValue<Scalar, ValueType>(values));
  }

private:
  StmtInterpreter& m_stmt_interpreter;

  std::vector<std::unique_ptr<Value>> m_stack;
};

template<typename Scalar>
Interpreter&
TypeInstancer<Scalar>::GetInterpreter()
{
  return m_expr_interpreter.GetInterpreter();
}

class StmtInterpreter final : public StmtVisitor
{
public:
  StmtInterpreter(Interpreter& interp)
    : m_interp(interp)
  {}

  Interpreter& GetInterpreter() { return m_interp; }

  void Visit(const AssignStmt& assign_stmt) override
  {
    ExprInterpreter expr_interpreter(*this);

    assign_stmt.GetExpr().Accept(expr_interpreter);

    m_interp.SetVar(assign_stmt.GetVarName(), expr_interpreter.PopValue());
  }

  void Visit(const ExprStmt& expr_stmt) override
  {
    ExprInterpreter expr_interpreter(*this);

    expr_stmt.GetExpr().Accept(expr_interpreter);
  }

  void Visit(const EmptyStmt&) override {}

private:
  Interpreter& m_interp;
};

Interpreter&
ExprInterpreter::GetInterpreter()
{
  return m_stmt_interpreter.GetInterpreter();
}

} // namespace

Interpreter::Interpreter(std::ostream& o_stream, std::ostream& e_stream)
  : m_out_stream(o_stream)
  , m_err_stream(e_stream)
{}

bool
Interpreter::Run(LineBuffer& line_buffer)
{
  while (line_buffer.GetAvailableLineCount() > 0) {

    std::string line = line_buffer.PopLine();

    Lexer lexer;

    lexer.Append(line.c_str(), line.size());

    Parser parser(lexer);

    auto stmt = parser.ParseStmt();
    if (!stmt)
      return false;

    StmtInterpreter stmt_interpreter(*this);

    try {
      stmt->Accept(stmt_interpreter);
    } catch (const std::exception& e) {
      m_err_stream << e.what() << std::endl;
      return false;
    }
  }

  return true;
}

} // namespace vision
