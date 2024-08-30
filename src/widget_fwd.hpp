#ifndef FLUXPP_WIDGET_FWD_HPP
#define FLUXPP_WIDGET_FWD_HPP

#include "basic.hpp"

namespace fluxpp{
  class BaseWidget;


  
  template<WidgetType>
  class DeferredWidget;

  

  template <WidgetType widget_type_, class return_t_  >
  class Widget;

  
  
  template <WidgetType widget_type_, class render_fn_t_, class render_sig_t_>
  class WidgetImpl;

  
}

#endif //FLUXPP_WIDGET_FWD_HPP


