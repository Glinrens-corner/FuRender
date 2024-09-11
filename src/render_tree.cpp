#include <memory>
#include <optional>
#include <tuple>
#include <assert.h>
#include <vector>

#include "id_types.hpp"
#include "render_tree.hpp"
#include "render_visitor.hpp"
#include "widget.hpp"


namespace fluxpp{




  RenderTree::RenderTree(
			 std::shared_ptr<DeferredWidget<WidgetType::Application> > root_widget,
			 State* state):state_(state){
    this->root_instance_ = widget_null_instance;

    //

    this->render_queue_.emplace_back(RenderQueueEntry{
	widget_null_instance,
	this->id_generator_.get_next_instance_id(),
	0,
	std::move(root_widget)});
  }



  widget_instance_id_t RenderTree::get_next_widget_to_update() const{
    auto instance_to_update_it = this->widget_instances_to_update_.begin();
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
    if (entry.parent_id != widget_null_instance){
      auto it = this->render_tree_.find(entry.parent_id);
      assert(it != this->render_tree_.end() && "parent of entry doesn't exists");
      // assert(std::find(  it->second.subinstances.begin(),it->second.subinstances.end(),entry.instance) != it->second.subinstances.end() && "instance not subinstance of parent");
    }
#endif //NDEBUG
    RenderVisitor visitor( this->state_ ,entry.key, entry.instance_id, entry.widget ,entry.parent_id , this);
    entry.widget->accept(visitor);
    if (entry.parent_id == widget_null_instance){
      this->root_instance_ = entry.instance_id;
    }
  }



  void RenderTree::rerender_instance(widget_instance_id_t new_instance_id){
    auto child_it = this->render_tree_.find(new_instance_id );
    
    assert(child_it != this->render_tree_.end() && "instance to rerender not found");
    explicit_key_t key;
    if(child_it->second.parent == widget_null_instance){
      key = 0;
    } else {
      auto parent_it = this->render_tree_.find(new_instance_id );
      
      assert(parent_it != this->render_tree_.end() && "no parent instance, but not root");
      auto mapping_it = std::find_if(parent_it->second.children.begin(),parent_it->second.children.end(), [new_instance_id](const std::pair<explicit_key_t, widget_instance_id_t >& pair){return pair.second == new_instance_id  ;});
      assert(mapping_it != parent_it->second.children.end() && "instance not a child of parent");
      key = mapping_it->first;
    }
    
    RenderVisitor visitor(this->state_, key, new_instance_id, child_it->second.widget, child_it->second.parent, this);
    child_it->second.widget->accept(visitor);
  }



  std::optional<WidgetInstanceData*> RenderTree::get_render_node_ptr(widget_instance_id_t id){
    auto it = this->render_tree_.find(id);
    if (it != this->render_tree_.end()){
      return &it->second;
    }else{
      return {};
    }
  }



  void RenderTree::empty_deletion_stack(){
    while(not this->deletion_stack_.empty()){
      widget_instance_id_t next_instance_id = this->deletion_stack_.back();
      this->deletion_stack_.pop_back();
      this->delete_instance(next_instance_id);
    }
  }



  void RenderTree::delete_instance(widget_instance_id_t instance_id){
    this->widget_instances_to_update_.erase(instance_id);
    auto it = this->render_tree_.find(instance_id);
    if(it != this->render_tree_.end()){
      for(std::size_t ipath=0; ; ipath++){
	
	std::optional<std::reference_wrapper<const std::string>>     path_opt = it->second.widget->get_nth_selector_address(ipath);
	if(path_opt.has_value()){
	  this->state_->remove_subscription( path_opt.value().get(),instance_id);
	}else {
	  break;
	}
	for (auto [key, child] : it->second.children){
	  (void)key;
	  this->deletion_stack_.push_back(child);	
	}
      }
      auto nerased = this->render_tree_.erase(instance_id);
      assert(nerased==1);
    }
  }


  
  WidgetInstanceData* RenderTree::insert_instance(widget_instance_id_t instance_id, WidgetInstanceData&& data){
    // Todo dispose of the old widgetInstance
    this->delete_instance(instance_id);
    
    auto [inserter , ok] = this->render_tree_.insert({instance_id,std::move(data)});
    assert(ok && "insertion of new widget failed");
    this->widget_instances_to_update_.erase(instance_id);
    for (std::size_t ipath=0 ; ;ipath++ ){
      std::optional<std::reference_wrapper<const std::string>> path_opt = inserter->second.widget->get_nth_selector_address(ipath);
      if(path_opt.has_value()){
	this->state_->accept_subscription(path_opt.value().get(), instance_id);
      }else {
	break;
      }
    }
    this->empty_deletion_stack();
    return &inserter->second;
  }



  void RenderTree::set_render_node(widget_instance_id_t id, WidgetInstanceData &&new_node){
    this->render_tree_[id] = std::move(new_node);
  }



  bool RenderTree::has_to_be_updated(widget_instance_id_t instance_id)const{
    auto it = this->widget_instances_to_update_.find(instance_id);
    return it != this->widget_instances_to_update_.end();
  }


  




}
