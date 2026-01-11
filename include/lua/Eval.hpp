#pragma once

#include <unordered_map>
#include <CTRPluginFramework/Menu/MenuEntry.hpp>

#include "AST.hpp"
#include "Object.hpp"

namespace CTRPluginFramework::lua {

class ASTEvaluator {

  SourceFile* source;

  MenuEntry* entry;

  std::unordered_map<StringID, Object*> globals;

  Object& get_global(StringID name) {
    auto& p = globals[name];

    if (!p)
      p = new Object(TypeKind::None);

    return *p;
  }

public:

  ASTEvaluator(SourceFile* source, MenuEntry* entry)
    : source(source),
      entry(entry)
  {
  }

  auto eval(ast::Program* prg) -> void {
    try {
      for(auto&&x:prg->codes){
        eval_stmt(x);
      }
    }
    catch (...) {
      OSD::Notify("Runtime error!");
      entry->Disable();
    }
  }

  auto eval_stmt(ast::Stmt* tree) -> void {
    switch(tree->kind)
    {
      case ast::StmtKind::Assign: {

        break;
      }

      case ast::StmtKind::If: {

        break;
      }
    }
  }

  auto eval_expr(ast::Expr* tree) -> Object {

  }

  auto eval_lvalue(ast::Expr* tree) -> Object* {

  }

};

}