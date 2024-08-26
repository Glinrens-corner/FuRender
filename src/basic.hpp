#ifndef FLUXPP_BASIC_HPP
#define FLUXPP_BASIC_HPP

#include <tuple>

namespace fluxpp{

  template <class T>
  struct error;
  
  template <class ... arg_ts_>
  using template_list = std::tuple <arg_ts_ ... >;  

  template <std::size_t i, class template_list_t>
  using template_list_element = std::tuple_element< i, template_list_t>;
  
  enum class WidgetType{
    Application,
    Client,
    Visual2D
  };
}

#endif //FLUXPP_BASIC_HPP
