#include <gtest/gtest.h>

#include <vision/interpreter.hpp>
#include <vision/line_buffer.hpp>

using namespace vision;

namespace {

class ValuePrinter final : public vision::ValueVisitor
{
public:
  ValuePrinter(std::ostream& out_stream)
    : m_out_stream(out_stream)
  {}

  void Visit(const Int& i) override { m_out_stream << i.value; }

  void Visit(const Float& f) override { m_out_stream << f.value; }

  void Visit(const Vec2& v) override
  {
    m_out_stream << '(' << v.x << ", " << v.y << ')';
  }

  void Visit(const Vec3& v) override
  {
    m_out_stream << '(' << v.x << ", " << v.y << ", " << v.z << ')';
  }

  void Visit(const Vec4& v) override
  {
    m_out_stream << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w
                 << ')';
  }

  void Visit(const Vec2i& v) override
  {
    m_out_stream << '(' << v.x << ", " << v.y << ')';
  }

  void Visit(const Vec3i& v) override
  {
    m_out_stream << '(' << v.x << ", " << v.y << ", " << v.z << ')';
  }

  void Visit(const Vec4i& v) override
  {
    m_out_stream << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w
                 << ')';
  }

private:
  std::ostream& m_out_stream;
};

class DebugFunc final : public vision::Function
{
public:
  DebugFunc(std::ostream& log_stream)
    : m_log_stream(log_stream)
  {}

  auto Call(const std::vector<const vision::Value*>& args)
    -> std::unique_ptr<Value> override
  {
    m_log_stream << "Debug:\n";

    for (const auto* value : args) {

      std::ostringstream arg_stream;

      ValuePrinter value_printer(arg_stream);

      value->Accept(value_printer);

      m_log_stream << "  - " << arg_stream.str() << std::endl;
    }

    return nullptr;
  }

private:
  std::ostream& m_log_stream;
};

} // namespace

TEST(Interpreter, MissingFunc)
{
  LineBuffer line_buffer;
  line_buffer.Write("debug()\n");
  line_buffer.Write("\n");

  std::ostringstream null_stream;

  Interpreter interpreter(null_stream, null_stream);

  EXPECT_EQ(interpreter.Run(line_buffer), false);
}

TEST(Interpreter, FuncCall)
{
  LineBuffer line_buffer;
  line_buffer.Write("debug(1, vec2i(3, 4))\n");
  line_buffer.Write("\n");

  Interpreter interpreter(std::cout, std::cerr);

  std::ostringstream debug_stream;

  interpreter.SetFunc("debug", new DebugFunc(debug_stream));

  EXPECT_EQ(interpreter.Run(line_buffer), true);

  auto out = debug_stream.str();

  EXPECT_EQ(out,
            "Debug:\n"
            "  - 1\n"
            "  - (3, 4)\n");
}

TEST(Interpreter, AssignStmt)
{
  LineBuffer line_buffer;
  line_buffer.Write("let a = 4\n");
  line_buffer.Write("\n");

  Interpreter interpreter(std::cout, std::cerr);

  EXPECT_EQ(interpreter.Run(line_buffer), true);

  const Value* value = interpreter.FindVar("a");

  ASSERT_NE(value, nullptr);

  std::ostringstream value_stream;

  ValuePrinter value_printer(value_stream);

  value->Accept(value_printer);

  EXPECT_EQ(value_stream.str(), "4");
}
