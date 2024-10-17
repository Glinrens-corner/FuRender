#ifndef FURENDER_CONTEXT_BASE_HPP
#define FURENDER_CONTEXT_BASE_HPP

#include <memory>
#include <vector>

#include "basic.hpp"
#include "client.hpp"
#include "widget_instance_data.hpp"

namespace furender{
  class RenderTree;
  class State;
  struct CollectingContext;


  template <WidgetType >
  class Context;

}



#endif // FURENDER_CONTEXT_BASE_HPP
