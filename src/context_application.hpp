#ifndef FLUXPP_CONTEXT_APPLICATION_HPP
#define FLUXPP_CONTEXT_APPLICATION_HPP

#include "context_base.hpp"

namespace fluxpp{

  template <>
  class Context<WidgetType::Application>{
  public:
    Context(RenderNode* , RenderTree* ){};
    
  };

}


#endif //FLUXPP_CONTEXT_APPLICATION_HPP
