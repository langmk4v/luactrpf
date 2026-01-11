#pragma once

#include "AST.hpp"

namespace CTRPluginFramework::lua {

class ASTEvaluator {

  SourceFile* source;

public:

  ASTEvaluator(SourceFile* source)
    : source(source)
  {
  }

  auto eval(ast::Program* prg) -> void {



  }


};

}