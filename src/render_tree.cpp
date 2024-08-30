#include <memory>
#include <tuple>
#include <assert.h>
#include <vector>

#include "id_types.hpp"
#include "render_tree.hpp"
#include "render_visitor.hpp"
#include "widget.hpp"


namespace fluxpp{
  widget_instance_id_t InstanceIdGenerator::get_next_instance_id(){
    widget_instance_id_t ret = this->current_instance_id;
    this->current_instance_id += 1;
    return ret;
  }


  
  
  RenderTree::RenderTree(
			 std::shared_ptr<DeferredWidget<WidgetType::Application> > root_widget,
			 State* state):state_(state){
    // root_instance refers to a node in render_tree
    // this instance is not yet set.
    this->root_instance_ = widget_null_instance;

    //
    
    this->render_queue_.emplace_back(RenderQueueEntry{
	widget_null_instance,
	this->id_generator_.get_next_instance_id(),
	std::move(root_widget)});
  }


  
  widget_instance_id_t RenderTree::get_next_widget_to_update() const{
    using iterator_t = typename decltype(this->widget_instances_to_update_)::iterator;
    iterator_t instance_to_update_it = this->widget_instances_to_update_.begin();
    {
	widget_instance_id_t parent = this->render_tree_.find(*instance_to_update_it)->second.parent;
	while(  parent != widget_null_instance ){
	  widget_instance_id_t current = parent;
	  auto update_it = this->widget_instances_to_update_.find(current ); 
	  if ( update_it != this->widget_instances_to_update_.end()){
	    instance_to_update_it = update_it;
	  }
	  parent = this->render_tree_.find(current)->second.parent;
	}
    }
    return *instance_to_update_it;
  }


  
  void RenderTree::do_render(){
    // only in the first render 
    while( !this->render_queue_.empty()  ){
      RenderQueueEntry& entry = this->render_queue_.back();
      this->render_render_queue_entry(entry);
      this->render_queue_.pop_back();
    }
    while(!this->widget_instances_to_update_.empty()){
      if(this->root_instance_ == widget_null_instance){
	assert(false && "error: no tree, but instances in the tree to update");
	// error: no tree, but instances in the tree to update???
	break;
      }
      
      widget_instance_id_t next_to_update =  this->get_next_widget_to_update();
      this->rerender_instance(next_to_update);
      
      while( !this->render_queue_.empty() ){
	RenderQueueEntry& entry = this->render_queue_.back();
	this->render_render_queue_entry(entry);
	this->render_queue_.pop_back();
      }
    }
  }


  
  void RenderTree::render_render_queue_entry(RenderQueueEntry &entry){

#ifndef NDEBUG
    if (entry.parent != widget_null_instance){
      auto it = this->render_tree_.find(entry.parent);
      assert(it != this->render_tree_.end() && "parent of entry doesn't exists");
      assert(std::find(  it->second.subinstances.begin(),it->second.subinstances.end(),entry.instance) != it->second.subinstances.end() && "instance not subinstance of parent");
    }
#endif //NDEBUG
    RenderNode node{entry.widget, entry.parent, std::vector<widget_id_t>{}, std::unique_ptr<ValueHolderBase>{}};
    RenderVisitor visitor( this->state_, &node, this);
    entry.widget->accept(visitor);
  }



  void RenderTree::rerender_instance(widget_instance_id_t instance_id){
    auto it = this->render_tree_.find(instance_id );
    assert(it != this->render_tree_.end() && "instance to rerender not found");
    RenderNode& node = it->second;
    RenderVisitor visitor(this->state_,&it->second, this);
    node.widget->accept(visitor);
  }

  
  
  std::pair<widget_instance_id_t,RenderNode*> RenderTree::add_new_render_node(RenderNode &&node){
    const widget_instance_id_t id =this->id_generator_.get_next_instance_id();
    
    auto [it, ok ] =  this->render_tree_.insert({id, std::move(node)});
    assert(ok);

    return std::make_pair(id, &it->second );
  };
}


