#ifndef FURENDER_ID_TYPES_HPP
#define FURENDER_ID_TYPES_HPP

#include <cstdint>


#include "basic.hpp"

namespace furender {

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
    struct command_type_id_tag_t;
    struct command_instance_id_tag_t;
    struct backend_instance_id_tag_t;
    struct client_instance_id_tag_t;
  }



  using command_type_id_t = detail::Id<detail::command_type_id_tag_t> ;
  using command_instance_id_t = detail::Id<detail::command_instance_id_tag_t> ;

  using backend_instance_id_t = detail::Id<detail::backend_instance_id_tag_t> ;
  using client_instance_id_t = detail::Id<detail::client_instance_id_tag_t> ;



  using widget_id_t = detail::Id<detail::widget_id_tag_t> ;
  using widget_instance_id_t = detail::Id<detail::widget_instance_id_tag_t> ;

  constexpr widget_instance_id_t widget_null_instance(0);


  using explicit_key_t = uint32_t ;
} // namespace furender



template <class tag_t_, class internal_t_>
struct std::hash<furender::detail::Id<tag_t_, internal_t_>>{
  std::size_t operator()(const furender::detail::Id<tag_t_, internal_t_>& id)const{
    return std::hash<internal_t_>{}(id.value());
  }
};


#endif //FURENDER_ID_TYPES_HPP
