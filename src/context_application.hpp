#ifndef FLUXPP_CONTEXT_APPLICATION_HPP
#define FLUXPP_CONTEXT_APPLICATION_HPP

#include "basic.hpp"
#include "id_types.hpp"
#include "context_base.hpp"
#include "render_tree.hpp"
#include "state.hpp"
#include "widget_fwd.hpp"
#include <memory>
#include <vector>

namespace fluxpp{

  template <>
  class Context<WidgetType::Application>{
  private:
    widget_instance_id_t current_instance_;    
    RenderNode * current_node_;
    RenderTree* tree_;
    State * state_;
    std::vector<widget_instance_id_t> rendered_subinstances_{};
    
  public:
    Context(widget_instance_id_t current_instance, RenderNode* current_node, RenderTree* tree, State* state ):
      current_instance_(current_instance),
      current_node_(current_node),
      tree_(tree),
      state_(state){
      (void) this->current_node_;// shut up the unused var warning
    };


    
    void render(std::shared_ptr<DeferredWidget<WidgetType::Client> >)const;

    
    
    template<class return_t>
    return_t immediate_render(std::shared_ptr<Widget<WidgetType::Client,  return_t> > widget){
      const RenderNode & child_node = render_i(std::move(widget));
      const ValueHolder<return_t>* p =  dynamic_cast<const ValueHolder<return_t>*>(child_node.return_value.get());
      if(p){
	return p->data;
      }
      return {};
    }
    


  private:
    const RenderNode& render_i(std::shared_ptr<DeferredWidget<WidgetType::Client> >);
  };
}




#endif //FLUXPP_CONTEXT_APPLICATION_HPP
