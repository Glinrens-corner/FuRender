#ifndef FURENDER_BASIC_HPP
#define FURENDER_BASIC_HPP

#include <tuple>

#ifndef NDEBUG
#define DEBUG_ONLY(x) x
#else
#define DEBUG_ONLY(x)
#endif

namespace furender {

  // helper class to debug template instanciation
  template <class T>
  struct error_s;

  // to debug template instanciation
  template <class T>
  using error = typename error_s<T>::type;

  template <class... arg_ts_>
  using template_list = std::tuple<arg_ts_...>;

  template <std::size_t i, class template_list_t>
  using template_list_element = std::tuple_element<i, template_list_t>;

  enum class None {
    none
  };

  enum class WidgetType {
    Application,
    Client,
    Window,
    Visual2D
  };
} // namespace furender

#endif // FURENDER_BASIC_HPP
