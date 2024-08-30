#ifndef FLUXPP_RENDER_TREE_HPP
#define FLUXPP_RENDER_TREE_HPP

#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_map>

#include "basic.hpp"
#include "value_holder.hpp"
#include "id_types.hpp"
#include "widget_fwd.hpp"



namespace fluxpp{
  class BaseWidget;
  template<WidgetType >
  class DeferredWidget;
  


  /** @brief struct that holds all information of a given widget instance 
   *
   */
  struct RenderNode{
  public:
    std::shared_ptr<BaseWidget> widget;
    //    std::vector<std::pair<widget_id_t, > >
    //    subinstances{};  // mapping (widget of subinstance, occurrence of that widget) -> instance
    std::unique_ptr<ValueHolderBase> return_value{};
  };


  
  class InstanceIdGenerator{
  private:
    widget_instance_id_t current_instance_id;
  public:
    InstanceIdGenerator(widget_instance_id_t id):current_instance_id(id){};

    widget_instance_id_t get_next_instance_id();
  };




  
  /** 
   *
   */
  class RenderTree{
  private:
    widget_instance_id_t root_instance_;
    std::unordered_map<widget_instance_id_t, RenderNode> render_tree_;
    InstanceIdGenerator id_generator_{1};
    //std::forward_list<> render_queue;
  public:
    RenderTree(std::shared_ptr<DeferredWidget<WidgetType::Application>> root_widget);

    std::pair<widget_instance_id_t, RenderNode*> add_new_render_node(RenderNode&& );
    

    void render_all();
  };
  
}

#endif //FLUXPP_RENDER_TREE_HPP
