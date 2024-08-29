#include "state.hpp"



namespace fluxpp{
  StateSlice * State::get_state_slice(const std::string &path){
    auto it = this->slices_.find(path);
    if (it != this->slices_.end()){
      return it->second.get();
    }else {
      return nullptr;
    }
  };

}

