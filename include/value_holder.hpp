#ifndef FLUXPP_VALUE_HOLDER_HPP
#define FLUXPP_VALUE_HOLDER_HPP

#include <memory>

namespace fluxpp{
  class ValueHolderBase{
  public:
    virtual ~ValueHolderBase(){};
  };

  template<class T>
  class ValueHolder:
    public ValueHolderBase{
  public:
    T data;
  public:
    ValueHolder(T data):data(std::move(data)){}
  };
} // namespace fluxpp

#endif //FLUXPP_VALUE_HOLDER_HPP
