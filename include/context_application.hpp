#ifndef FURENDER_CONTEXT_APPLICATION_HPP
#define FURENDER_CONTEXT_APPLICATION_HPP

#include <memory>
#include <vector>



#include "basic.hpp"

#include "id_types.hpp"

#include "context_base.hpp"
#include "render_tree.hpp"
#include "state.hpp"
#include "value_holder.hpp"
#include "widget_fwd.hpp"

namespace furender{

  template <>
  class Context<WidgetType::Application>{
  private:
    widget_instance_id_t instance_id_; // id of the instance the context is used when rendering
    std::optional<WidgetInstanceData *> old_data_; // pointer to data of previous rendering of the instance
    RenderTree* tree_;
    State * state_;
    std::vector<widget_instance_id_t> orphaned_children_{}; // child instances that where previously rendered, but not rendered anymore. (these instances  have to be deleted afterwards, as they are now unreachable from root. )
    std::vector<std::pair<explicit_key_t,widget_instance_id_t> > rendered_children_{}; // child instances being rendered.

  public:
    Context(widget_instance_id_t instance_id, std::optional<WidgetInstanceData*> old_data, RenderTree* tree, State* state ):
      instance_id_(instance_id),
      old_data_(old_data),
      tree_(tree),
      state_(state){
      if (this->old_data_.has_value()){
	for (auto [key, instance]: this->old_data_.value()->children ){
	  (void) key;
	  this->orphaned_children_.push_back(instance);
	}
      }
    };



    void render(explicit_key_t key, std::shared_ptr<DeferredWidget<WidgetType::Client> >)const;



    template<class return_t>
    return_t immediate_render(explicit_key_t key, std::shared_ptr<Widget<WidgetType::Client,  return_t> > widget){

      // instance_id and data of previous rendering
      std::optional<std::pair<widget_instance_id_t, WidgetInstanceData*> > old_child_data = this->get_old_instance_data(key, *widget.get());

      // if there is old data, it is the same instance so reuse the instance id
      widget_instance_id_t instance_id =
	old_child_data.has_value()
	? old_child_data.value().first
	: this->tree_->get_next_instance_id();


      //erase the id of the next rendered child_instance from the array of orphaned children.
      // only necessary if the instance_id is reused. otherwise it was newly created and was not used before.
      if (old_child_data.has_value()){
	auto it = std::find(this->orphaned_children_.begin(),
			    this->orphaned_children_.end(), instance_id);
	if(it != this->orphaned_children_.end()){
	  this->orphaned_children_.erase(it);
	}
      }


      // if the old instance is still current, return the value of the last execution.
      if (old_child_data.has_value()){
	if (this->subinstance_is_current( *widget, instance_id,  *old_child_data.value().second) ){
	  ValueHolderBase*base = old_child_data.value().second->return_value.get();
	  ValueHolder<return_t>* derived = dynamic_cast<ValueHolder<return_t>*  >(base);
	  assert(derived && "same widget, but different return type ?? that shouldn't be possible" );
	  return derived->data;
	}
      }
      // otherwise we need to render the child
      const WidgetInstanceData & child_node = render_i(key,instance_id, std::move(widget));
      const ValueHolder<return_t>* p =  dynamic_cast<const ValueHolder<return_t>*>(child_node.return_value.get());
      if(p){
	return p->data;
      } else {
	assert(false && "a Widget< _ ,  return_t> must return a value of type return_t");
	//return {};
      }
    }


    /** @brief get the child_instances the parent widget tried to render (some may have been current from the previous render)
     *
     */
    std::vector<std::pair<explicit_key_t,widget_instance_id_t> > get_children(){
      return std::move(this->rendered_children_);
    }


    /** @brief get the ids of the children of the last rendered parent instance, that aren't used anymore
     *
     */
    const std::vector<widget_instance_id_t>  get_orphaned_children(){
      return std::move(this->orphaned_children_);
    }
  private:

    // fully specialized part of the render method.
    const WidgetInstanceData& render_i(explicit_key_t key,widget_instance_id_t child_id, std::shared_ptr<DeferredWidget<WidgetType::Client> > widget);

    // determine if there is a
    std::optional<std::pair<widget_instance_id_t, WidgetInstanceData*> > get_old_instance_data(explicit_key_t key, const BaseWidget& widget) const;

    // determine if the child instance can be reused or has to be rerendered.
    bool subinstance_is_current(const BaseWidget&, widget_instance_id_t, const WidgetInstanceData&)const;
  };
}




#endif //FURENDER_CONTEXT_APPLICATION_HPP
