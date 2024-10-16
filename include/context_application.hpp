#ifndef FURENDER_CONTEXT_APPLICATION_HPP
#define FURENDER_CONTEXT_APPLICATION_HPP

#include <memory>
#include <vector>



#include "basic.hpp"
#include "collecting_context.hpp"
#include "id_types.hpp"
#include "context_base.hpp"
#include "render_tree.hpp"
#include "state.hpp"
#include "value_holder.hpp"
#include "widget_fwd.hpp"
#include "widget_instance_data.hpp"
#include "renderer.hpp"

namespace furender{

  template <>
  class Context<WidgetType::Application>{
  private:
    CollectingContext * collecting_context_= nullptr;
    RenderTree* tree_;
    State * state_;

  public:
    Context(CollectingContext* collecting_context,
	    RenderTree* tree,
	    State* state ):
      collecting_context_(collecting_context),
      tree_(tree),
      state_(state){
    };


    /** @brief renders a widget
     *
     * is allowed to defer the rendering of the widget therefore it doesn't return the returnvalue of the widget.
     *
     * @param key a key to map the widget.
     * @param widget the widget to render.
     */
    // not yet implemented
    //    void render(explicit_key_t key, std::shared_ptr<DeferredWidget<WidgetType::Client> > widget)const;


    /**@brief renders a widget and returns its return value
     *
     * @param key a key to map the widget.
     * @param widget the widget to render.
     */
    template<class return_t>
    return_t immediate_render(explicit_key_t key, std::shared_ptr<Widget<WidgetType::Client,  return_t> > widget){

      // creating a new CollectingContext for the newly rendered context.
      std::unique_ptr<CollectingContext> child_collecting_context_uptr = std::make_unique<CollectingContext>() ;
      CollectingContext* child_collecting_context = child_collecting_context_uptr.get();
      this->collecting_context_->new_instance_data.value().subcontexts.push_back({key, std::move(child_collecting_context_uptr)});


      detail::Renderer(this->tree_, this->state_).render_internal(key,this->collecting_context_->instance_id,std::move(widget),child_collecting_context);

      ValueHolderBase* base = child_collecting_context->old_instance_data.has_value()
	? child_collecting_context->old_instance_data.value()->return_value.get()
	: child_collecting_context->new_instance_data.value().return_value.get();

      const ValueHolder<return_t>* derived =  dynamic_cast<const ValueHolder<return_t>*>(base);
      if(derived){
	return derived->data;
      } else {
	// this should be impossible by the typesystem.
	assert(false && "a Widget< _ ,  return_t> must return a value of type return_t");
	//return {};
      }
    }



  };
}




#endif //FURENDER_CONTEXT_APPLICATION_HPP
