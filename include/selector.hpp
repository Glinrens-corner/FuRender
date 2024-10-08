#ifndef FLUXPP_SELECTOR_HPP
#define FLUXPP_SELECTOR_HPP

#include <string>
#include <type_traits>

namespace fluxpp{
  /** @brief Class that communicates the path and elementtype of a StateSlice to a Widget
   *
   *  @tparam selected_t_ type of the value of the  selected state, must be default constructible.
   */
  template<class selected_t_ >
  class Selector{
  public:
    using selected_t = selected_t_;
  private:
    std::string path_ {};
  public:
    /** @brief constructor of the Selector
     *
     * @param path string that describes the path to the StateSlice
     */
    Selector(std::string path):path_(std::move(path)){}



    /** @brief get the path to the state
     */
    const std::string & path() const{
      return this->path_;
    }
  private:
    static_assert( std::is_default_constructible<selected_t_>::value ,"State must be default constructible" );
  };

}

#endif // FLUXPP_SELECTOR_HPP
