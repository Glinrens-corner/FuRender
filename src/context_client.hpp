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
    widget_instance_id_t current_instance_;
    WidgetInstanceData * current_node_;
    RenderTree* tree_;
    State * state_;
    std::vector<widget_instance_id_t> rendered_subinstances_{};

  public:
    Context(widget_instance_id_t current_instance, WidgetInstanceData* current_node, RenderTree* tree, State* state ):
      current_instance_(current_instance),
      current_node_(current_node),
      tree_(tree),
      state_(state){
      // shut up the unused var warning
      (void) this->current_instance_;
      (void) this->current_node_;
    };



    void render(std::shared_ptr<DeferredWidget<WidgetType::Visual2D>> widget){
      (void) this->tree_;
      (void) this->current_node_;
      (void)this->state_;
    };
  };

}

#endif // FLUXPP_CONTEXT_CLIENT_HPP
