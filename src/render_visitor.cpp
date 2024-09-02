#include "render_visitor.hpp"
#include "id_types.hpp"
#include "render_tree.hpp"
#include "widget.hpp"

namespace fluxpp{
  WidgetInstanceData* RenderVisitor::get_node_ptr(){

    std::optional<WidgetInstanceData*> node_optr = this->tree_->get_render_node_ptr( this->instance_id_);
    if (node_optr.has_value()){
      return node_optr.value();
    } else {

      this->node_opt_ = WidgetInstanceData{this->widget_, this->parent_id_, std::vector<widget_instance_id_t>{},std::unique_ptr<ValueHolderBase>{}};
      return &this->node_opt_.value();
    }
  }



  void RenderVisitor::store_render_node(){
    if(this->node_opt_.has_value()){
      this->tree_->set_render_node(this->instance_id_, std::move(this->node_opt_.value()));
      for ( int iselector = 0; this->widget_->get_nth_selector_address( iselector).has_value(); iselector++ ){

	this->tree_->get_state().accept_subscription(this->widget_->get_nth_selector_address( iselector).value().get(), this->instance_id_);
      }
    } else {

    }

  }


}
