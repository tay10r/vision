#pragma once

#include <vision/expr.hpp>
#include <vision/stmt.hpp>
#include <vision/token.hpp>
#include <vision/type_id.hpp>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace vision {

class Lexer;

class Parser final
{
public:
  Parser(Lexer& lexer);

  auto ParseStmt() -> std::unique_ptr<Stmt>;

  /// @note Only exposed for testing, not meant to be called directly.
  auto ParseExpr() -> std::unique_ptr<Expr>;

  auto Remaining() const noexcept -> size_t;

private:
  auto ParseExprList(char l_sym, char r_sym)
    -> std::optional<std::vector<std::unique_ptr<Expr>>>;

  auto ParseAssignStmt() -> std::unique_ptr<Stmt>;

  auto ParseExprStmt() -> std::unique_ptr<Stmt>;

  auto ParseEmptyStmt() -> std::unique_ptr<Stmt>;

  auto ParseIntExpr() -> std::unique_ptr<Expr>;

  auto ParseCallExpr() -> std::unique_ptr<Expr>;

  auto ParseTypeConstructor() -> std::unique_ptr<Expr>;

  auto GetTypeID(const std::optional<Token>& token) const
    -> std::optional<TypeID>;

  auto Peek(size_t offset) const noexcept -> std::optional<Token>;

  void Advance(size_t count);

  struct Memo final
  {
    size_t token_offset = 0;
  };

  auto MakeMemo() const noexcept -> Memo;

  void Restore(const Memo&);

private:
  std::vector<Token> m_tokens;

  size_t m_token_offset = 0;

  std::map<std::string, TypeID, std::less<>> m_type_map;
};

} // namespace vision
