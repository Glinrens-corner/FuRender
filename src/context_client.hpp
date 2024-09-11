#ifndef FLUXPP_CONTEXT_CLIENT_HPP
#define FLUXPP_CONTEXT_CLIENT_HPP

#include <memory>
#include <optional>
#include <vector>

#include "basic.hpp"
#include "id_types.hpp"
#include "context_base.hpp"
#include "widget_fwd.hpp"

namespace fluxpp{

  template<>
  class Context<WidgetType::Client>{
  private:
    widget_instance_id_t instance_id_;
    std::optional<WidgetInstanceData *> old_data_;
    RenderTree* tree_;
    State * state_;

    std::vector<std::pair<explicit_key_t,widget_instance_id_t> > rendered_subinstances_{};

  public:
    Context(widget_instance_id_t instance_id, std::optional<WidgetInstanceData*> old_data, RenderTree* tree, State* state ):
      instance_id_(instance_id),
      old_data_(old_data),
      tree_(tree),
      state_(state){
    };



    void render(explicit_key_t key, std::shared_ptr<DeferredWidget<WidgetType::Visual2D> >)const;

    template<class return_t>
    return_t immediate_render(explicit_key_t key, std::shared_ptr<Widget<WidgetType::Client,  return_t> > widget){
      return {};
    }


    std::vector<std::pair<explicit_key_t,widget_instance_id_t> > get_children(){
      return this->rendered_subinstances_;
    }



    const std::vector<widget_instance_id_t>  get_orphaned_children(){
      return {};
    }

  };

}

#endif // FLUXPP_CONTEXT_CLIENT_HPP
