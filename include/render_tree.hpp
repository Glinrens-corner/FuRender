#ifndef FLUXPP_RENDER_TREE_HPP
#define FLUXPP_RENDER_TREE_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <hash_map>

#include "basic.hpp"
#include "value_holder.hpp"

namespace fluxpp{
  using widget_instance_id_t = uint64_t ;
  class BaseWidget;
  
  class RenderNode{
  public:
    std::shared_ptr<BaseWidget> widget;
    std::vector<widget_instance_id_t> subinstances{}; 
    std::unique_ptr<ValueHolderBase> return_value{};
  };
  
  class RenderTree{
  private:
    widget_instance_id_t root_instance_;
    std::unordered_map<widget_instance_id_t, RenderNode> render_tree_;
  public:
    
  };
  
}

#endif //FLUXPP_RENDER_TREE_HPP
