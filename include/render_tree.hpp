#ifndef FLUXPP_RENDER_TREE_HPP
#define FLUXPP_RENDER_TREE_HPP

#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_map>
#include <deque>
#include <set>

#include "basic.hpp"
#include "state.hpp"
#include "value_holder.hpp"
#include "id_types.hpp"
#include "widget_fwd.hpp"



namespace fluxpp{


  /** @brief struct that holds all information of a given widget instance 
   *
   */
  struct RenderNode{
  public:
    std::shared_ptr<BaseWidget> widget;
    widget_instance_id_t parent;
    std::vector<widget_instance_id_t > 
    subinstances{};  // mapping (widget of subinstance, occurrence of that widget) -> instance
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
  
  /*
   *
   */
  class RenderTree{

  private:
    struct RenderQueueEntry{
      widget_instance_id_t parent ;
      widget_instance_id_t instance;
      std::shared_ptr<BaseWidget> widget;
    };
  private:
    widget_instance_id_t root_instance_;
    std::unordered_map<widget_instance_id_t, RenderNode> render_tree_;
    InstanceIdGenerator id_generator_{1};
    State* state_=nullptr;
    std::deque<RenderQueueEntry> render_queue_{};
    std::set<widget_instance_id_t> widget_instances_to_update_{};
    std::vector<widget_instance_id_t> search_stack_{};
  public:
    RenderTree(std::shared_ptr<DeferredWidget<WidgetType::Application>> root_widget,
	       State* state);
    
    std::pair<widget_instance_id_t, RenderNode*> add_new_render_node(RenderNode&& );
    
    void do_render();


  public:
    // widget_instance_id_t get_null_instance(){
    //   return widget_null_instance;
    // };


    
  private:
    void do_render_internal();

    void render_render_queue_entry(RenderQueueEntry&);

    widget_instance_id_t get_next_widget_to_update()const;

    void rerender_instance(widget_instance_id_t);
  };
  
}

#endif //FLUXPP_RENDER_TREE_HPP
