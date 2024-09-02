#ifndef FLUXPP_CONTEXT_VISUAL2D_HPP
#define FLUXPP_CONTEXT_VISUAL2D_HPP

#include "context_base.hpp"

namespace fluxpp{

  template <>
  class Context<WidgetType::Visual2D>{
  public:
    Context(WidgetInstanceData* , RenderTree* ){};

  };

}


#endif //FLUXPP_CONTEXT_VISUAL2D_HPP
