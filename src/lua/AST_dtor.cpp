#include "lua/AST.hpp"

namespace CTRPluginFramework::lua::ast {

CallFunc::~CallFunc(){
  if(functor)delete functor;
  for(auto x:args)delete x;
}

Terms::~Terms() {
  for (auto& t : this->terms)
    delete t.second;
}

Assign::~Assign() {
  if (this->dest) delete this->dest;
  if (this->source) delete this->source;
}

Scope::~Scope() {
  for(auto&& x:codes)
    delete x;
}

If::~If()
{
  if(cond)delete cond;
  if(body)delete body;
  if(elseif)delete elseif;
  if(else_body)delete else_body;
}

Func::~Func()
{
  if(body)delete body;
}

}