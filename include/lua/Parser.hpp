#pragma once

#include <functional>
#include <string>

#include "AST.hpp"

#include "Errors.hpp"
#include "SourceFile.hpp"

#include "Logger.hpp"

namespace CTRPluginFramework::lua {

struct SourceFile;

class Parser {
  using Kwd = TokKeywords;
  using Expr = ast::Expr;
  using Stmt = ast::Stmt;

  SourceFile *source;

  Token *cur;

public:
  Parser(SourceFile *source) : source(source), cur(source->token) {}

  bool is_end() const { return cur->is_eof(); }

  void next() { this->cur = this->cur->next; }

  bool look(auto cond) { return cur->is(cond); }

  bool eat(auto cond) { return look(cond) && (next(), true); }

  Token *expect(auto cond) {
    if (!look(cond)) {
      source->add_error(Error(cur, "unexpected token"));
      return nullptr;
    }

    return next(), cur->prev;
  }

  bool eat_open_of(TokBrackets b) { return cur->is(b, true) && (next(), true); }

  bool eat_close_of(TokBrackets b) {
    return cur->is(b, false) && (next(), true);
  }

  Token *expect_open_of(TokBrackets b) {
    if (!cur->is(b, true)) {
      source->add_error(Error(cur, "unexpected token"));
      return nullptr;
    }

    return next(), cur->prev;
  }

  Token *expect_close_of(TokBrackets b) {
    if (!cur->is(b, false)) {
      source->add_error(Error(cur, "unexpected token"));
      return nullptr;
    }

    return next(), cur->prev;
  }

  Expr *p_factor() {
    auto tok = cur;

    if (eat(Kwd::True)) {
      auto obj = new Object(TypeKind::Bool);
      obj->v_bool = true;
      return new ast::Value(tok, obj);
    }

    if (eat(Kwd::False)) {
      return new ast::Value(tok, new Object(TypeKind::Bool));
    }

    if (eat(TokenKind::Literal)) {
      auto val = new ast::Value(tok);

      switch (tok->literal) { case TokenLiterals::I32: }

      return val;
    }

    if (eat(TokenKind::Identifier))
      return new ast::Variable(tok);

    Logger::Emit(
        Utils::Format("cur=%p,kind=%d,str=", cur, static_cast<int>(cur->kind)) +
        string(cur->get_strview()));

    return nullptr;
  }

  auto p_primary() -> Expr * {
    auto x = p_factor();
    if (!x)
      return nullptr;

    if (auto B = cur; eat_open_of(TokBrackets::Normal)) {
      auto cf = new ast::CallFunc(x, B);

      if (eat_close_of(TokBrackets::Normal))
        return cf;

      do {
        if (auto arg = p_expr())
          cf->args.push_back(arg);
        else
          return (delete cf), nullptr;
      } while (eat(TokPunctuators::Comma));

      if (!expect_close_of(TokBrackets::Normal))
        return (delete cf), nullptr;

      return cf;
    }

    return x;
  }

  Expr *p_mul();

  Expr *p_add() {
    auto x = p_primary();
    if (!x)
      return nullptr;

    while (!is_end()) {
      auto op = cur;
      if (eat(TokOperators::Add)) {
        if (auto y = p_primary())
          x = ast::Terms::make(ast::ExprKind::Add, op, x, y);
        else
          return (delete x), nullptr;
      } else if (eat(TokOperators::Sub)) {
        if (auto y = p_primary())
          x = ast::Terms::make(ast::ExprKind::Sub, op, x, y);
        else
          return (delete x), nullptr;
      } else
        break;
    }

    return x;
  }

  Expr *p_shift() {
    auto x = p_add();
    if (!x)
      return nullptr;

    while (!is_end()) {
      auto op = cur;
      if (eat(TokOperators::LShift)) {
        if (auto y = p_add())
          x = ast::Terms::make(ast::ExprKind::LShift, op, x, y);
        else
          return (delete x), nullptr;
      } else if (eat(TokOperators::RShift)) {
        if (auto y = p_add())
          x = ast::Terms::make(ast::ExprKind::RShift, op, x, y);
        else
          return (delete x), nullptr;
      } else
        break;
    }

    return x;
  }

  Expr *p_compare();

  Expr *p_equality();

  Expr *p_bit_and() {
    auto x = p_shift();
    if (!x)
      return nullptr;

    while (!is_end()) {
      auto op = cur;
      if (eat(TokOperators::BitAnd)) {
        if (auto y = p_shift())
          x = ast::Terms::make(ast::ExprKind::BitAnd, op, x, p_shift());
        else
          return (delete x), nullptr;
      } else
        break;
    }

    return x;
  }

  Expr *p_bit_xor();

  Expr *p_bit_or();

  Expr *p_log_and();

  Expr *p_log_or();

  Expr *p_expr() { return p_bit_and(); }

  auto p_ifs_else(Token *elsetok) -> ast::Scope * {
    auto body = new ast::Scope(elsetok);

    while (!is_end()) {
      if (eat(Kwd::End)) {
        break;
      }
      if (auto x = p_stmt())
        body->codes.push_back(x);
      else {
        delete body;
        return nullptr;
      }
    }

    return body;
  }

  auto p_ifs_body(ast::If *ifs) -> bool {
    while (!is_end()) {
      if (eat(Kwd::End)) {
        return true;
      }
      if (auto tok = cur; eat(Kwd::Elseif)) {
        if (!(ifs->elseif = p_ifs(tok)))
          return false;
        return true;
      }
      if (auto elsetok = cur; eat(Kwd::Else)) {
        if (!(ifs->else_body = p_ifs_else(elsetok)))
          return false;
        return true;
      }
      if (auto x = p_stmt())
        ifs->body->codes.push_back(x);
      else
        return false;
    }
    source->add_error(Error(ifs->token, "if statement not terminated."));
    return false;
  }

  auto p_ifs(Token *iftok) -> ast::If * {
    if (auto if_cond = p_expr()) {
      if (auto tok_Then = expect(Kwd::Then)) {
        auto ifs = new ast::If(iftok);
        ifs->cond = if_cond;
        ifs->body = new ast::Scope(tok_Then);
        if (p_ifs_body(ifs)) {
          return ifs;
        } else {
          delete ifs;
          return nullptr;
        }
      } else {
        source->add_error(
            Error(cur->prev, "expected 'then' after this token."));
        delete if_cond;
      }
    } else {
      source->add_error(Error(iftok, "expected expression after this token."));
    }
    return nullptr;
  }

  auto p_stmt() -> Stmt * {

    auto tok = cur;

    if (eat(Kwd::If)) {
      return p_ifs(tok);
    }

    if (auto x = p_expr()) {
      if (auto t = cur; eat(TokOperators::Assign)) {
        if (auto src = p_expr()) {
          return new ast::Assign(x, src, t);
        } else {
          delete x;
          source->add_error(Error(t, "expected expression after this token."));
          return nullptr;
        }
      }

      return new ast::ExprStatement(x);
    } else {
      source->add_error(Error(cur, "expected statement or expression."));
      return nullptr;
    }
  }

  auto p_function() -> ast::Func * {
    auto decltok = expect(Kwd::Fn);

    if (!decltok)
      return nullptr;

    auto nametok = expect(TokenKind::Identifier);
    if (!nametok)
      return nullptr;

    if (!expect_open_of(TokBrackets::Normal))
      return nullptr;

    std::vector<Token *> argnames;

    if (!eat_close_of(TokBrackets::Normal)) {
      do {
        if (!argnames.emplace_back(expect(TokenKind::Identifier)))
          return nullptr;
      } while (eat(TokPunctuators::Comma));

      if (!expect_close_of(TokBrackets::Normal))
        return nullptr;
    }

    // TODO

    return nullptr;
  }

  auto parse(Token *token) -> ast::Program * {

    this->cur = token;

    auto prg = new ast::Program();

    while (!is_end()) {
      if (look(TokKeywords::Fn)) {
        if (auto x = p_function())
          prg->append_func(x);
        else
          goto __fail;
      } else {
        if (auto x = p_stmt())
          prg->append_stmt(x);
        else
          goto __fail;
      }
    }

    return prg;

  __fail:
    delete prg;
    return nullptr;
  }
};

} // namespace CTRPluginFramework::lua