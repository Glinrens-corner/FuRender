#ifndef FLUXPP_ADDRESSOR_HPP
#define FLUXPP_ADDRESSOR_HPP
#include <string>

namespace fluxpp{
  /** @brief Class to record state, accepted events and path of a state slice
   *
   */
  template <class state_t_, class ... event_ts_>
  class Addressor{
  private:
    std::string path_;
    
  public:
    Addressor(std::string path):path_(std::move( path_)){}


    
    const std::string& path()const{
      return this->path_;
    };
    
  };


}

#endif //FLUXPP_ADDRESSOR_HPP
