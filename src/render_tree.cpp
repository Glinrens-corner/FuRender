#include <memory>
#include <tuple>
#include <assert.h>

#include "id_types.hpp"
#include "render_tree_fwd.hpp"
#include "widget.hpp"


namespace fluxpp{
  widget_instance_id_t InstanceIdGenerator::get_next_instance_id(){
    widget_instance_id_t ret = this->current_instance_id;
    this->current_instance_id += 1;
    return ret;
  }


  
  RenderTree::RenderTree(std::shared_ptr<DeferredWidget<WidgetType::Application> > root_widget){
    RenderNode node {std::move(root_widget)};
    widget_instance_id_t id = this->id_generator_.get_next_instance_id();
    this->root_instance_ = id;
    this->render_tree_.insert({id, std::move(node)});
  }
  

  std::pair<widget_instance_id_t,RenderNode*> RenderTree::add_new_render_node(RenderNode &&node){
    const widget_instance_id_t id =this->id_generator_.get_next_instance_id();
    
    auto [it, ok ] =  this->render_tree_.insert({id, std::move(node)});
    assert(ok);

    return std::make_pair(id, &it->second );
  };
}


