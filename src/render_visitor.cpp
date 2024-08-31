#include "render_visitor.hpp"
#include "render_tree.hpp"


namespace fluxpp{
  RenderNode* RenderVisitor::get_node_ptr(){
    
    std::optional<RenderNode*> node_optr = this->tree_->get_render_node_ptr( this->instance_id_);
    if (node_optr.has_value()){
      return node_optr.value();
    } else {
      
      this->node_opt_ = RenderNode{this->widget_, this->parent_id_, std::vector<widget_id_t>{},std::unique_ptr<ValueHolderBase>{}};
      return &this->node_opt_.value();
    }
  }



  void RenderVisitor::store_render_node(){
    if(this->node_opt_.has_value()){
      this->tree_->set_render_node(this->instance_id_, std::move(this->node_opt_.value()));
    }
    
  }

  
}
