#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace vision {

class LineBuffer;

struct Int;
struct Float;
struct Vec2;
struct Vec3;
struct Vec4;
struct Vec2i;
struct Vec3i;
struct Vec4i;

class ValueVisitor
{
public:
  virtual ~ValueVisitor() = default;

  virtual void Visit(const Int&) = 0;

  virtual void Visit(const Float&) = 0;

  virtual void Visit(const Vec2&) = 0;
  virtual void Visit(const Vec3&) = 0;
  virtual void Visit(const Vec4&) = 0;

  virtual void Visit(const Vec2i&) = 0;
  virtual void Visit(const Vec3i&) = 0;
  virtual void Visit(const Vec4i&) = 0;
};

struct Value
{
  virtual ~Value() = default;

  virtual void Accept(ValueVisitor&) const = 0;
};

struct Int final : public Value
{
  int value;

  Int(int v)
    : value(v)
  {}

  void Accept(ValueVisitor& v) const override { v.Visit(*this); }
};

struct Float final : public Value
{
  float value;

  Float(float v)
    : value(v)
  {}

  void Accept(ValueVisitor& v) const override { v.Visit(*this); }
};

struct Vec2 final : public Value
{
  float x;
  float y;

  Vec2(float a, float b)
    : x(a)
    , y(b)
  {}

  void Accept(ValueVisitor& v) const override { v.Visit(*this); }
};

struct Vec3 final : public Value
{
  float x;
  float y;
  float z;

  Vec3(float a, float b, float c)
    : x(a)
    , y(b)
    , z(c)
  {}

  void Accept(ValueVisitor& v) const override { v.Visit(*this); }
};

struct Vec4 final : public Value
{
  float x;
  float y;
  float z;
  float w;

  Vec4(float a, float b, float c, float d)
    : x(a)
    , y(b)
    , z(c)
    , w(d)
  {}

  void Accept(ValueVisitor& v) const override { v.Visit(*this); }
};

struct Vec2i final : public Value
{
  int x;
  int y;

  Vec2i(int a, int b)
    : x(a)
    , y(b)
  {}

  void Accept(ValueVisitor& v) const override { v.Visit(*this); }
};

struct Vec3i final : public Value
{
  int x;
  int y;
  int z;

  Vec3i(int a, int b, int c)
    : x(a)
    , y(b)
    , z(c)
  {}

  void Accept(ValueVisitor& v) const override { v.Visit(*this); }
};

struct Vec4i final : public Value
{
  int x;
  int y;
  int z;
  int w;

  Vec4i(int a, int b, int c, int d)
    : x(a)
    , y(b)
    , z(c)
    , w(d)
  {}

  void Accept(ValueVisitor& v) const override { v.Visit(*this); }
};

class Interpreter;

class Function
{
public:
  virtual ~Function() = default;

  virtual auto Call(const std::vector<const Value*>& args)
    -> std::unique_ptr<Value> = 0;
};

class RenderFunction : public Function
{
public:
  virtual ~RenderFunction() = default;

  auto Call(const std::vector<const Value*>& args) -> std::unique_ptr<Value>;

  virtual void Call(const Vec2i& pixel_count,
                    const Vec2i& pixel_offset,
                    const Vec2i& pixel_stride) = 0;
};

class Interpreter final
{
public:
  Interpreter(std::ostream& out_stream, std::ostream& err_stream);

  void SetFunc(const std::string_view& name, Function* func)
  {
    m_func_map[std::string(name)] = std::unique_ptr<Function>(func);
  }

  auto FindFunc(const std::string_view& name) -> Function*
  {
    auto it = m_func_map.find(name);
    if (it == m_func_map.end())
      return nullptr;
    else
      return it->second.get();
  }

  void SetVar(const std::string_view& name, std::unique_ptr<Value>&& expr)
  {
    m_var_map[std::string(name)] = std::move(expr);
  }

  auto FindVar(const std::string_view& name) const -> const Value*
  {
    auto it = m_var_map.find(name);
    if (it == m_var_map.end())
      return nullptr;
    else
      return it->second.get();
  }

  bool Run(LineBuffer&);

private:
  std::map<std::string, std::unique_ptr<Function>, std::less<>> m_func_map;

  std::map<std::string, std::unique_ptr<Value>, std::less<>> m_var_map;

  std::ostream& m_out_stream;

  std::ostream& m_err_stream;
};

} // namespace vision
