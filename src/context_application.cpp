#include "context_application.hpp"
#include "id_types.hpp"
#include "render_visitor.hpp"
#include <type_traits>
#include "widget.hpp"

namespace fluxpp{
  void Context<WidgetType::Application>::render(std::shared_ptr<DeferredWidget<WidgetType::Client> > widget)const{
    //    widget_id_t widget_id = widget->get_widget_id();
    
  }


  
  const RenderNode& Context<WidgetType::Application>::render_i(std::shared_ptr<DeferredWidget<WidgetType::Client>>  widget){
    RenderNode node{widget};
    RenderVisitor visitor(this->state_,&node, this->tree_  );

    widget->accept(visitor);
    auto [new_instance_id, node_ptr] = this->tree_->add_new_render_node(std::move(node) );

    return *node_ptr;
  }

}
