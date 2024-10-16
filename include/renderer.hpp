#ifndef FURENDER_RENDERER_HPP
#define FURENDER_RENDERER_HPP



#include "id_types.hpp"
#include "render_tree.hpp"
#include "state.hpp"
#include "collecting_context.hpp"

namespace furender{
  namespace detail{
    class Renderer{
    private:
      RenderTree* render_tree_;
      State* state_;
    public:
      Renderer(RenderTree* render_tree, State* state):
	render_tree_(render_tree),
	state_(state){}

      // determine if there is a record for this child widget instance from a previous render.
      // note: is called in render and therefore before we have determined the children instances id
      // indeed if there is an old instance this determines that instances id which will be reused.
      std::optional<std::pair<widget_instance_id_t, WidgetInstanceData*> > get_old_instance_data(explicit_key_t key, widget_instance_id_t parent ) const;

      // non templated part of the render function.
      // The collecting_context is essentially an inout parameter.
      // but the widget is also transferred via the collecting context
      void render_internal(explicit_key_t key,
			   widget_instance_id_t parent_id,
			   std::shared_ptr<BaseWidget> widget,
			   CollectingContext* collecting_context);

      // determine if the child instance can be reused or has to be rerendered.
      bool subinstance_is_current(const BaseWidget&, widget_instance_id_t, const WidgetInstanceData&)const;

    };
  }
}



#endif //FURENDER_RENDERER_HPP
