#ifndef FURENDER_VALUE_HOLDER_HPP
#define FURENDER_VALUE_HOLDER_HPP

#include <memory>
#include <type_traits>

namespace furender{

  /** @brief base class to hold an arbitrary value
   *
   */
  /* to detect if it holds the expected type (value_t)
   * do:
   * if(ValueHolder<value_t>* p = dynamic_cast<ValueHolder<value_t>*>( value_holder_base); p){
   *}
   */
  class ValueHolderBase{
  public:
    virtual ~ValueHolderBase(){};
  };





  /** @brief holds a value of any specific type
   *
   * @tparam value_t_ type of the held value (must be move_constructible)
   */
  template<class value_t_>
  class ValueHolder:
    public ValueHolderBase{
  public:
    using value_t = value_t_;
  public:
    value_t_ data;
  public:
    ValueHolder(value_t_ data):data(std::move(data)){}

  private:
    static_assert(std::is_move_constructible_v<value_t_>, "value must be move constructible");
  };
} // namespace furender

#endif //FURENDER_VALUE_HOLDER_HPP
