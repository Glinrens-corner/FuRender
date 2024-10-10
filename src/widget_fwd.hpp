#ifndef FURENDER_WIDGET_FWD_HPP
#define FURENDER_WIDGET_FWD_HPP

#include "basic.hpp"

namespace furender{
  class BaseWidget;



  template<WidgetType>
  class DeferredWidget;



  template <WidgetType widget_type_, class return_t_  >
  class Widget;



  template <WidgetType widget_type_, class render_fn_t_, class render_sig_t_>
  class WidgetImpl;


}

#endif //FURENDER_WIDGET_FWD_HPP
