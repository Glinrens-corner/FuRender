#ifndef FLUXPP_CONTEXT_HPP
#define FLUXPP_CONTEXT_HPP

#include "context_base.hpp"
#include "basic.hpp"

namespace fluxpp{

  template<>
  class Context<WidgetType::Client>{
  public:
    Context(RenderNode* , RenderTree* ){};
    
  };

}

#endif // FLUXPP_CONTEXT_HPP
