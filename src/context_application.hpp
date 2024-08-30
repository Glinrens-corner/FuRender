#ifndef FLUXPP_CONTEXT_APPLICATION_HPP
#define FLUXPP_CONTEXT_APPLICATION_HPP

#include "basic.hpp"
#include "id_types.hpp"
#include "context_base.hpp"
#include "render_tree_fwd.hpp"
#include "state.hpp"
#include "widget_fwd.hpp"
#include <memory>
#include <vector>

namespace fluxpp{

  template <>
  class Context<WidgetType::Application>{
  private:
    RenderNode* current_node_;
    RenderTree* tree_;
    State * state_;
    std::vector<std::pair<widget_id_t, uint16_t>> seen_widgets; // which widget has been seen how often.
  public:
    Context(RenderNode* current_node, RenderTree* tree, State* state ):
      current_node_(current_node),
      tree_(tree),
      state_(state){
      (void) this->current_node_;
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
