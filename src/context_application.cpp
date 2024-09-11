#include "context_application.hpp"
#include "id_types.hpp"
#include "render_visitor.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>
#include <utility>

#include "widget.hpp"

namespace fluxpp{
  void Context<WidgetType::Application>::render(explicit_key_t key, std::shared_ptr<DeferredWidget<WidgetType::Client> > widget)const{
    assert(false&& "render is not yet implemented");
    
  }


  
  bool Context<WidgetType::Application>::subinstance_is_current(const BaseWidget& widget, widget_instance_id_t instance_id, const WidgetInstanceData& data)const{
    return (widget.get_widget_id() == data.widget->get_widget_id()
	    and (not this->tree_->has_to_be_updated(instance_id)));
  }


  const WidgetInstanceData& Context<WidgetType::Application>::render_i(explicit_key_t key, widget_instance_id_t child_id, std::shared_ptr<DeferredWidget<WidgetType::Client>>  widget){
    BaseWidget* widget_ptr = widget.get();
    RenderVisitor visitor(this->state_, key,child_id, std::move(widget) , this->instance_id_, this->tree_  );

    widget_ptr->accept(visitor);

    WidgetInstanceData* node_ptr = visitor.get_child_instance_data_ptr();
    this->rendered_children_.push_back({key, child_id});
    assert(node_ptr);
    return *node_ptr;
  }

    
  std::optional<std::pair<widget_instance_id_t, WidgetInstanceData*> > Context<WidgetType::Application>::get_old_instance_data(explicit_key_t key, const BaseWidget& ) const{
    auto  it = std::find_if(this->rendered_children_.begin(),
		 this->rendered_children_.end(),
		 [key](const std::pair<explicit_key_t,widget_instance_id_t> &pair){
		   return pair.first == key;
		 });
    if (it!=this->rendered_children_.end()){
      auto node_optr = this->tree_->get_render_node_ptr(it->second);
      assert(node_optr.has_value());
      return  std::make_pair(it->second,node_optr.value());
    }else {
      return {};
    }

  }
}
