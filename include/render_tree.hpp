#ifndef FURENDER_RENDER_TREE_HPP
#define FURENDER_RENDER_TREE_HPP

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_map>
#include <deque>
#include <unordered_set>

#include "basic.hpp"
#include "collecting_context.hpp"
#include "state.hpp"
#include "value_holder.hpp"
#include "id_types.hpp"
#include "widget_fwd.hpp"
#include "widget_instance_data.hpp"


namespace furender{


  /** @brief generator for widget_instance_ids
   *
   */
  class InstanceIdGenerator{
  private:
    widget_instance_id_t::value_t current_instance_value_;
  public:
    InstanceIdGenerator(widget_instance_id_t::value_t id):current_instance_value_(id){};

    widget_instance_id_t get_next_instance_id(){
      return widget_instance_id_t( this->current_instance_value_++);
    };
  };



  /** @brief generator for client_instance_ids
   *
   */
  class ClientIdGenerator{
  private:
    client_instance_id_t::value_t current_instance_value_;
  public:
    ClientIdGenerator(client_instance_id_t::value_t  id):current_instance_value_(id){};

    client_instance_id_t get_next_instance_id(){
      return client_instance_id_t( this->current_instance_value_++);
    };
  };






  /** @brief holder class for the widget or render tree.
   *
   */
  class RenderTree{

  private:
    struct RenderQueueEntry{
      widget_instance_id_t parent_id ;
      widget_instance_id_t instance_id;
      explicit_key_t key;
      std::shared_ptr<BaseWidget> widget;
    };
#ifdef NDEBUG
  public:
    using debug_render_queue_entry_t = RenderQRenderQueueEntry;
#endif
  private:
    std::optional<widget_instance_id_t> root_instance_{};
    std::unordered_map<widget_instance_id_t, WidgetInstanceData> render_tree_;
    InstanceIdGenerator widget_instance_id_generator_{1};
    ClientIdGenerator client_id_generator_{1};
    State* state_=nullptr;
    std::deque<RenderQueueEntry> render_queue_{};
    std::unordered_set<widget_instance_id_t> widget_instances_to_update_{};
    std::unordered_set<widget_instance_id_t> deletion_set_{};
    std::shared_ptr<DeferredWidget<WidgetType::Application>> root_widget_;
  public:
    RenderTree(std::shared_ptr<DeferredWidget<WidgetType::Application>> root_widget,
	       State* state);

    /** @brief render all widgets in the render tree that have to be rendered.
     *
     */
    void render_all();

    /** @brief add a freshly rendered instance to the render_tree.
     *
     *
     */
    WidgetInstanceData* insert_instance(widget_instance_id_t , WidgetInstanceData&&);

    /** @brief get the pointer to the data of an instance.
     *
     * if the return  has a value, it is dereferencable.
     */
    std::optional<WidgetInstanceData*> get_widget_instance_data(widget_instance_id_t);

    /** @brief check if an instance has to be updated
     *
     */
    bool has_to_be_updated(widget_instance_id_t instance_id)const;

    void set_render_node(widget_instance_id_t id, WidgetInstanceData&& new_node);

    void delete_instance(widget_instance_id_t );

    void empty_deletion_stack();
  public:
    /** @brief get a new @c widget_instance_id_t  
     *
     */
    widget_instance_id_t get_next_widget_instance_id(){
      return this->widget_instance_id_generator_.get_next_instance_id();
    }


    /** /brief announce that a property (e.g. StateSlice) a widget instance depends upon has changed
     *
     */
    void announce_change(widget_instance_id_t instance_id){
      this->widget_instances_to_update_.insert(instance_id);
    }



    State& get_state(){
      return *this->state_;
    }



#ifndef NDEBUG
    std::optional<widget_instance_id_t> debug_get_root_instance(){
	return this->root_instance_;
    }
#endif



#ifndef NDEBUG
    const std::unordered_map<widget_instance_id_t, WidgetInstanceData>&
    debug_get_render_tree()const{
      return this->render_tree_;
    }
#endif



#ifndef NDEBUG
    const std::deque<RenderQueueEntry> &
    debug_get_render_queue()const{
      return this->render_queue_;
    }
#endif



#ifndef NDEBUG
    const std::unordered_set<widget_instance_id_t>& debug_get_widget_instances_to_update()const{
      return this->widget_instances_to_update_;
    }
#endif
  private:
    void render_root(CollectingContext*);

    //    void do_render_internal();

    void render_render_queue_entry(RenderQueueEntry&);

    widget_instance_id_t get_next_widget_to_update()const;

    void rerender_instance(widget_instance_id_t, CollectingContext*);

    void merge_collecting_context(CollectingContext& );

    /** @brief inserts or updates instance_data for a widget
     *
     *  - updates all subscriptions to state_slices
     *  - marks all orphaned children for deletion
     *  - does not change or check the parent child relationships.
     */
    void insert_or_update_instance(WidgetInstanceData&&, std::optional<WidgetInstanceData*> );

    /** @brief remove all subscriptions for a widget instance from state
     *
     */
    void remove_subscriptions_from_state(const WidgetInstanceData& );

    /** @brief subscribes an instance to all relevant stateslices 
     *
     */
    void subscribe_to_state(const WidgetInstanceData& );
  };

}

#endif //FURENDER_RENDER_TREE_HPP
