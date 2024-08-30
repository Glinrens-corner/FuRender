#ifndef FLUXPP_CONTEXT_CLIENT_HPP
#define FLUXPP_CONTEXT_CLIENT_HPP

#include <memory>
#include <vector>

#include "basic.hpp"
#include "id_types.hpp"
#include "context_base.hpp"
#include "widget_fwd.hpp"

namespace fluxpp{

  template<>
  class Context<WidgetType::Client>{
  private:
    RenderNode* current_node_;
    RenderTree* tree_;
    State * state_;
    std::vector<std::pair<widget_id_t, uint16_t>> seen_widgets; // which widget has been seen how often.
  public:
    Context(RenderNode* current_node, RenderTree* tree, State* state ):
      current_node_(current_node),
      tree_(tree),
      state_(state){};
    


    void render(std::shared_ptr<DeferredWidget<WidgetType::Visual2D>> widget){
      (void) this->tree_;
      (void) this->current_node_;
      (void)this->state_;
    };
  };

}

#endif // FLUXPP_CONTEXT_CLIENT_HPP
