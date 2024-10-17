#ifndef FURENDER_CONTEXT_BASE_HPP
#define FURENDER_CONTEXT_BASE_HPP

#include "basic.hpp"
#include "client.hpp"
#include "widget_instance_data.hpp"

#include <memory>
#include <vector>

namespace furender {
  class RenderTree;
  class State;
  struct CollectingContext;

  template <WidgetType>
  class Context;

} // namespace furender

#endif // FURENDER_CONTEXT_BASE_HPP
