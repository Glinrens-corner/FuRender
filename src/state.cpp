#include "state.hpp"
#include "id_types.hpp"
#include "render_tree.hpp"


namespace furender{
  StateSlice * State::get_state_slice(const std::string &path){
    auto it = this->slices_.find(path);
    if (it != this->slices_.end()){
      return it->second.slice.get();
    }else {
      return nullptr;
    }
  };



  void State::dispatch_event(EventEnvelope&envelope){
    auto it = this->slices_.find(envelope.path);
    if (it != this->slices_.end()){
      StateDispatchVisitor visitor(envelope);
      bool updated = it->second.slice->accept_dispatch_visitor(visitor);
      if(updated){
	for (const widget_instance_id_t& instance_id: it->second.subscriptions){

          this->render_tree_->announce_change(instance_id) ;
	}
      }
    }else {
      // Error, dispatch not found
    }

  }
}
