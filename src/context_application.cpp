#include "context_application.hpp"
#include "id_types.hpp"
#include "render_visitor.hpp"
#include <cassert>
#include <type_traits>

#include "widget.hpp"

namespace fluxpp{
  void Context<WidgetType::Application>::render(std::shared_ptr<DeferredWidget<WidgetType::Client> > widget)const{
    //    widget_id_t widget_id = widget->get_widget_id();

  }



  const WidgetInstanceData& Context<WidgetType::Application>::render_i(std::shared_ptr<DeferredWidget<WidgetType::Client>>  widget){


    widget_instance_id_t child_instance_id = this->tree_->get_next_instance_id() ;
    RenderVisitor visitor(this->state_, child_instance_id, widget , this->current_instance_, this->tree_  );

    widget->accept(visitor);

    std::optional<WidgetInstanceData*> node_optr = this->tree_->get_render_node_ptr(child_instance_id);
    assert(node_optr.has_value());
    return *node_optr.value();
  }

}
