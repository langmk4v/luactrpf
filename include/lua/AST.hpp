#pragma once

#include <vector>

#include "Token.hpp"
#include "ASTFwd.hpp"

namespace CTRPluginFramework::lua::ast {

enum class Kind {
  Expr,
  Stmt,
  Func,
};

enum class ExprKind {
  Value,
  Variable,
  CallFunc,
  Mul,
  Div,
  Mod,
  Add,
  Sub,
  LShift,
  RShift,
  Less,
  Greater,
  Equal,
  NotEqual,
  BitAnd,
  BitXor,
  BitOr,
  LogAnd,
  LogOr,
};

enum class StmtKind {
  Assign,
  Expr,
  Scope,
  If,
  For,
  Do,
  Break,
  Return,
};

struct Tree {
  Kind kind;
  Token* token;

  template <typename T>
  T* as() { return (T*)this; }

  template <typename T>
  T const* as() const { return (T const*)this; }

  bool is(Kind k) const { return kind == k; }

  virtual bool is_terms() const { return false; }

  virtual ~Tree() = default;

protected:
  Tree(Kind kind, Token* token)
    : kind(kind),
      token(token)
  {
  }
};

struct Expr : public Tree {
  ExprKind kind;

  bool is_terms() const override {
    return ExprKind::Mul <= kind && kind <= ExprKind::LogOr;
  }

  virtual ~Expr() = default;

protected:
  Expr(ExprKind kind, Token* token)
    : Tree(Kind::Expr, token),
      kind(kind)
  {
  }
};

struct Stmt : public Tree {
  StmtKind kind;

  bool is(StmtKind k) const { return kind == k; }

  virtual ~Stmt() = default;

protected:

  Stmt(StmtKind k, Token* tok)
    : Tree(Kind::Stmt, tok),
      kind(k)
  {
  }
};

struct Value final : public Expr {
  Value(Token* token)
    : Expr(ExprKind::Value, token)
  {
  }
};

struct Variable final : public Expr {
  StringID name;

  Variable(Token* token)
    : Expr(ExprKind::Variable, token),
      name(token->str)
  {
  }
};

struct CallFunc final : public Expr {
  Expr* functor;
  std::vector<Expr*> args;

  ~CallFunc();
  CallFunc(Expr* f,Token*B)
    :Expr(ExprKind::CallFunc,B),functor(f){}
};

struct Terms final : public Expr {
  Expr* base;
  std::vector<std::pair<Token*, Expr*>> terms;

  Expr* append(Token* op, Expr* item) {
    return terms.emplace_back(op, item).second;
  }

  static Terms* make(ExprKind kind, Token* op, Expr* lhs, Expr* rhs) {
    if (lhs->is_terms()) {
      lhs->as<Terms>()->append(op, rhs);
      return lhs->as<Terms>();
    }

    auto x = new Terms(kind, op, lhs);

    x->append(op, rhs);

    return x;
  }

  ~Terms();

  Terms(ExprKind kind, Token* op, Expr* base)
    : Expr(kind, op),
      base(base)
  {
  }
};

struct ExprStatement final : public Stmt {
  Expr* expr;

  ~ExprStatement() { if(expr)delete expr; }

  ExprStatement(Expr* e) : Stmt(StmtKind::Expr, e->token),expr(e) { }
};

struct Assign final : public Stmt {
  Expr*   dest;
  Expr*   source;

  ~Assign();

  Assign(Expr* dest, Expr* source, Token* op)
    : Stmt(StmtKind::Assign, op),
      dest(dest),
      source(source)
  {
  }
};

struct Scope final : public Stmt {
  std::vector<Stmt*> codes;

  ~Scope();

  Scope(Token* tok) : Stmt(StmtKind::Scope, tok) { }
};

struct If final : public Stmt {
  Expr* cond = nullptr;
  Scope* body = nullptr;
  If* elseif = nullptr;
  Scope* else_body = nullptr;

  ~If();

  If(Token* tok)
    : Stmt(StmtKind::If,tok)
  {
  }
};

struct Func final : public Tree {
  Token*  name_tok;
  std::vector<Token*> args;
  Token*  result_type;
  Scope* body=  nullptr;

  ~Func();

  Func(Token* decl)
    : Tree(Kind::Func, decl)
  {
  }
};

struct Program final {
  std::vector<Func*> functions;

  std::vector<Stmt*> codes;

  auto append_func(Func* f) -> Func* {
    return functions.emplace_back(f);
  }

  auto append_stmt(Stmt* s) -> Stmt* {
    return codes.emplace_back(s);
  }

  Program()
  {
  }

  ~Program()
  {
    for (auto x : functions)
      delete x;

    for (auto x : codes)
      delete x;
  }
};

}