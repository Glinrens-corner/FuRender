#ifndef FLUXPP_RENDER_TREE_HPP
#define FLUXPP_RENDER_TREE_HPP

#include <functional>
#include <memory>
#include <optional>
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

#ifdef NDEBUG
  private:
#else
  public:
#endif
    struct RenderQueueEntry{
      widget_instance_id_t parent ;
      widget_instance_id_t instance;
      std::shared_ptr<BaseWidget> widget;
    };
#ifdef NDEBUG
  private:
#else
  public:
#endif
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

    std::optional<RenderNode*> get_render_node_ptr(widget_id_t);

    void set_render_node(widget_instance_id_t id, RenderNode&& new_node);

  public:
    widget_instance_id_t get_next_instance_id(){
      return this->id_generator_.get_next_instance_id();
    }


    
    void announce_change(widget_instance_id_t instance_id){
      this->widget_instances_to_update_.insert(instance_id);
    }



    State& get_state(){
      return *this->state_;
    }



    void rendered_instance(widget_instance_id_t instance){
      this->widget_instances_to_update_.erase(instance);
    }
  private:
    void do_render_internal();

    void render_render_queue_entry(RenderQueueEntry&);

    widget_instance_id_t get_next_widget_to_update()const;

    void rerender_instance(widget_instance_id_t);
  };
  
}

#endif //FLUXPP_RENDER_TREE_HPP
