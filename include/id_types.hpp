#ifndef FLUXPP_ID_TYPES_HPP
#define FLUXPP_ID_TYPES_HPP

#include <cstdint>
#include <iostream>

#include "basic.hpp"

namespace fluxpp {

  namespace detail{
    template <class tag_t_, class internal_t_=uint64_t  >
    class Id{
    private:
      template <int i> // to avoid an recursive type
      using self_t = Id<tag_t_, internal_t_>;
    public:
      using value_t = internal_t_;
    private:
      internal_t_ value_;
    public:
      Id()=default;


      explicit constexpr Id(internal_t_ value):
	value_(value){}




      self_t<0>& operator = (const self_t<0>& other){
	this->value_ = other.value_;
	return *this;
      }



      bool operator==(const Id<tag_t_,internal_t_ >& other)const{
	return value_ == other.value_;
      }



      bool operator!=(const Id<tag_t_,internal_t_ >& other)const{
	return value_ != other.value_;
      }



      const internal_t_& value()const{
	return value_;
      }
    };



    struct widget_id_tag_t ;
    struct widget_instance_id_tag_t ;
  }



  using command_type_id = uint64_t ;
  using command_instance_id = uint64_t ;



  using widget_id_t = detail::Id<detail::widget_id_tag_t> ;
  using widget_instance_id_t = detail::Id<detail::widget_instance_id_tag_t> ;

  constexpr widget_instance_id_t widget_null_instance(0);
} // namespace fluxpp



template <class tag_t_, class internal_t_>
struct std::hash<fluxpp::detail::Id<tag_t_, internal_t_>>{
  std::size_t operator()(const fluxpp::detail::Id<tag_t_, internal_t_>& id)const{
    return std::hash<internal_t_>{}(id.value());
  }
};


#endif //FLUXPP_ID_TYPES_HPP
