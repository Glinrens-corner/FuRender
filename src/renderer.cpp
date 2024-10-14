
#include "renderer.hpp"
#include "id_types.hpp"
#include "widget.hpp"

namespace furender{
  namespace detail{
    void Renderer::render_internal(explicit_key_t key,
				   widget_instance_id_t parent_id,
				   CollectingContext* collecting_context){

      BaseWidget* widget_ptr = collecting_context->widget.get();

      std::optional<std::pair<widget_instance_id_t,WidgetInstanceData*>> old_data = this->get_old_instance_data( key, parent_id);

      // if there is old data, it is the same instance so reuse the instance id
      widget_instance_id_t instance_id =
	old_data.has_value()
	? old_data.value().first
	: this->render_tree_->get_next_widget_instance_id();
      collecting_context->instance_id = instance_id;


      // set old instance data.
      collecting_context->old_instance_data =
	old_data.has_value()
	? old_data.value().second
	: std::optional<WidgetInstanceData*>{};

      // if the old instance is still current, return the value of the last execution.
      if (old_data.has_value()){
	if (this->subinstance_is_current( *widget_ptr, instance_id,  *old_data.value().second) ){
	  assert(collecting_context->old_instance_data.has_value() && "if old_data has a value, collecting_context->old_instance_data should be set");
	  collecting_context->is_old_data_current = true;
	  return ;
	}
      }


      RenderVisitor visitor(this->state_,
			    this->render_tree_,
			    collecting_context
			    );
      // finally render the widget.
      widget_ptr->accept(visitor);

      return ;
    }



    std::optional<std::pair<widget_instance_id_t, WidgetInstanceData*> > Renderer::get_old_instance_data(explicit_key_t key, widget_instance_id_t parent_id ) const{
      auto parent_optr = this->render_tree_->get_render_node_ptr(parent_id);
      if (not parent_optr.has_value()) return {};
      WidgetInstanceData* parent_ptr = parent_optr.value();

      auto  it
	= std::find_if(parent_ptr->children.begin(),
		       parent_ptr->children.end(),
		       [key](const std::pair<explicit_key_t,widget_instance_id_t> &pair){
			 return pair.first == key;
		       });
      if (it != parent_ptr->children.end()){
	auto child_optr = this->render_tree_->get_render_node_ptr(it->second);
	assert(child_optr.has_value() && "parent is in tree but child not?");
	return  std::make_pair(it->second,child_optr.value());
      }else {
	return {};
      }

    }


    bool Renderer::subinstance_is_current(const BaseWidget& widget, widget_instance_id_t instance_id, const WidgetInstanceData& data)const{
      return (widget.get_widget_id() == data.widget->get_widget_id()
	      and (not this->render_tree_->has_to_be_updated(instance_id)));
    }

  }
}
