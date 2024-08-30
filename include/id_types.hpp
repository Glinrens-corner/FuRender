#ifndef FLUXPP_ID_TYPES_HPP
#define FLUXPP_ID_TYPES_HPP

#include <cstdint>

namespace fluxpp {
  
  using command_type_id = uint64_t ; 
  using command_instance_id = uint64_t ; 

  using widget_id_t = uint64_t ;
  using widget_instance_id_t = uint64_t ;

  constexpr widget_instance_id_t widget_null_instance = 0;
}


#endif //FLUXPP_ID_TYPES_HPP
