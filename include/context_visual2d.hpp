#ifndef FURENDER_CONTEXT_VISUAL2D_HPP
#define FURENDER_CONTEXT_VISUAL2D_HPP

#include "context_base.hpp"

namespace furender{

  template <>
  class Context<WidgetType::Visual2D>{
  public:
    Context(WidgetInstanceData* , RenderTree* ){};

  };

}


#endif //FURENDER_CONTEXT_VISUAL2D_HPP
