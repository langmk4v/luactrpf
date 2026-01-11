#pragma once

#include <algorithm>

#include <CTRPluginFramework/Menu/MenuEntry.hpp>

#include "AST.hpp"
#include "Object.hpp"

namespace CTRPluginFramework::lua {

class ASTEvaluator {
  using ExprKind = ast::ExprKind;
  using StmtKind = ast::StmtKind;

  struct VarPairMap {
    StringID  name;
    Object*   object;

    VarPairMap(StringID n, Object* o)
      : name(n), object(o) { }
  };

  struct VarStorage {
    std::vector<VarPairMap> storage;

    auto find(StringID name) {
      return std::find_if(storage.begin(), storage.end(), [name] (VarPairMap& map) -> bool {
        return map.name == name;
      });
    }

    auto contains(StringID name) -> bool {
      return find(name) != storage.end();
    }

    auto get(StringID name) -> Object*& {
      if (auto iter = find(name); iter != storage.end())
        return iter->object;

      return append(name).object;
    }

    auto append(StringID name, Object* obj = nullptr) -> VarPairMap& {
      return storage.emplace_back(name, obj);
    }
  };

  SourceFile *source;

  MenuEntry *entry;

  VarStorage globals;

  Object *get_global(StringID name)
  {
    if (!globals.contains(name)) {
      return
        globals.append(name, new Object(TypeKind::None))
          .object;
    }

    return globals.get(name);
  }

 public:
  ASTEvaluator(SourceFile *source, MenuEntry *entry)
      : source(source), entry(entry)
  {
  }

  auto eval(ast::Program *prg) -> void
  {
    try {
      for (auto &&x : prg->codes) {
        eval_stmt(x);
      }
    }
    catch (...) {
      OSD::Notify("Runtime error!");
      entry->Disable();
    }
  }

  auto eval_stmt(ast::Stmt *tree) -> void
  {
    switch (tree->kind) {
      case StmtKind::Assign: {
        auto x = tree->as<ast::Assign>();
        auto dest = eval_lvalue(x->dest);

        *dest = eval_expr(x->source);

        break;
      }

      case StmtKind::Scope: {
        auto x = tree->as<ast::Scope>();

        for (auto &&y : x->codes) eval_stmt(y);

        break;
      }

      case StmtKind::If: {
        auto x = tree->as<ast::If>();

        if (eval_expr(x->cond).v_bool)
          eval_stmt(x->body);
        else if (x->elseif)
          eval_stmt(x->elseif);
        else if (x->else_body)
          eval_stmt(x->else_body);

        break;
      }

      case StmtKind::Expr: {
        eval_expr(tree->as<ast::ExprStatement>()->expr);
        break;
      }
    }
  }

  auto eval_expr(ast::Expr *tree) -> Object
  {
    switch (tree->kind) {
      case ExprKind::Value:
        return *(tree->as<ast::Value>()->obj);

      case ExprKind::Variable:
        return *eval_lvalue(tree);

      case ExprKind::CallFunc: {
        auto cf = tree->as<ast::CallFunc>();

        std::vector<Object> args;

        for (auto &&x : cf->args)
          args.push_back(eval_expr(x));

        Object result;

        if (cf->functor->is(ExprKind::Variable)) {
          auto name = cf->functor->token->get_strview();

          if (name == "readf") {
            result.type = TypeKind::Float;
            Process::ReadFloat(args[0].v_u32, result.v_float);
            return result;
          }
          else if (name == "writef") {
            u32 addr = args[0].v_u32;
            float val = args[1].v_float;
            result.type = TypeKind::Bool;
            result.v_bool = Process::WriteFloat(addr, val);
            return result;
          }
          else if (name == "is_pressed") {
            u32 key = args[0].v_u32;
            result.type = TypeKind::Bool;
            result.v_bool = key != 0 & Controller::IsKeysDown(key);
            return result;
          }
          else if (name == "check_addr") {
            u32 addr = args[0].v_u32;
            result.type = TypeKind::Bool;
            result.v_bool = Process::CheckAddress(addr);
            return result;
          }
          else if (name == "notify") {
            std::string tmp;
            for (auto &&x : args) tmp += x.to_str();
            OSD::Notify(tmp);
            return result;
          }
          else if (name == "on_enabled") {
            result.type = TypeKind::Bool;
            result.v_bool = this->entry->WasJustActivated();
            return result;
          }
        }

        return {};
      }

      default: {
        if (tree->is_terms()) {
          auto expr = tree->as<ast::Terms>();

          auto val = eval_expr(expr->base);

          for (auto &&[op, term] : expr->terms) {
            auto x = eval_expr(term);
            switch (expr->kind) {
              case ExprKind::Add:
                add_object(val, x);
                break;
              case ExprKind::Sub:
                sub_object(val, x);
                break;
              case ExprKind::LShift:
                left_shift_object(val, x);
                break;
              case ExprKind::BitAnd:
                bit_and_object(val, x);
                break;
              case ExprKind::BitOr:
                bit_or_object(val, x);
                break;

              default:
                alert;
                Process::ReturnToHomeMenu();
            }
          }

          return val;
        }
        break;
      }
    }

    return {};
  }

  auto eval_lvalue(ast::Expr *tree) -> Object *
  {
    switch (tree->kind) {
      case ExprKind::Variable:
        return get_global(tree->as<ast::Variable>()->name);
      default:
        alert;
        Logger::Emit(
            Utils::Format("tree->kind = %d", static_cast<int>(tree->kind)));
        break;
    }
    return nullptr;
  }

  auto add_object(Object &a, Object &b) -> Object &
  {
    switch (a.type.kind) {
      case TypeKind::I32:
        a.v_i32 += b.v_i32;
        break;

      case TypeKind::U32:
        a.v_u32 += b.v_u32;
        break;

      case TypeKind::Float:
        a.v_float += b.v_float;
        break;
    }
    return a;
  }

  auto sub_object(Object &a, Object &b) -> Object &
  {
    switch (a.type.kind) {
      case TypeKind::I32:
        a.v_i32 -= b.v_i32;
        break;

      case TypeKind::U32:
        a.v_u32 -= b.v_u32;
        break;

      case TypeKind::Float:
        a.v_float -= b.v_float;
        break;
    }
    return a;
  }

  auto left_shift_object(Object &a, Object &b) -> Object &
  {
    switch (a.type.kind) {
      case TypeKind::I32:
        a.v_i32 <<= b.v_i32;
        break;
      case TypeKind::U32:
        a.v_u32 <<= b.v_u32;
        break;
    }
    return a;
  }

  auto bit_and_object(Object &a, Object &b) -> Object &
  {
    switch (a.type.kind) {
      case TypeKind::I32:
        a.v_i32 &= b.v_i32;
        break;
      case TypeKind::U32:
        a.v_u32 &= b.v_u32;
        break;
    }
    return a;
  }

  auto bit_or_object(Object &a, Object &b) -> Object &
  {
    switch (a.type.kind) {
      case TypeKind::I32:
        a.v_i32 |= b.v_i32;
        break;
      case TypeKind::U32:
        a.v_u32 |= b.v_u32;
        break;
    }
    return a;
  }
};

}  // namespace CTRPluginFramework::lua