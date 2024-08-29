#ifndef FLUXPP_BASIC_HPP
#define FLUXPP_BASIC_HPP

#include <tuple>

namespace fluxpp{

  
  
  // helper class to debug template instanciation
  template <class T>
  struct error_s;

  // to debug template instanciation
  template <class T>
  using error =  typename error_s<T>::type;

  
  template <class ... arg_ts_>
  using template_list = std::tuple <arg_ts_ ... >;  

  template <std::size_t i, class template_list_t>
  using template_list_element = std::tuple_element< i, template_list_t>;

  enum class None{
    none
  };
  
  enum class WidgetType{
    Application,
    Client,
    Visual2D
  };
}

#endif //FLUXPP_BASIC_HPP
