
#include "render_visitor.hpp"
#include "id_types.hpp"
#include "render_tree.hpp"
#include "widget.hpp"

namespace furender{
  void RenderVisitor::set_old_data(){
    this->old_data_  = this->tree_->get_render_node_ptr( this->instance_id_);
  }



  void RenderVisitor::finalize(){
    this->final_data_ptr_ = this->tree_->insert_instance(this->instance_id_,std::move(this->new_data_) );
  }


}
