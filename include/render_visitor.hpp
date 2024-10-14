
#ifndef FURENDER_RENDER_VISITOR_HPP
#define FURENDER_RENDER_VISITOR_HPP

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <optional>

#include <iostream>
#include <vector>

#include "basic.hpp"
#include "context.hpp"

#include "context_base.hpp"
#include "id_types.hpp"
#include "render_tree.hpp"
#include "value_holder.hpp"
#include "collecting_context.hpp"


#include "state.hpp"

namespace furender {

  namespace detail{
    /* classes to generate a sequence of integers
     *
     */
    template <int... >
    struct sequence{};



    template<int first, int ...numbers>
    struct sequence_generator: sequence_generator<first-1,first-1,numbers...>{};



    template <int ... numbers>
    struct sequence_generator<0, numbers...> {
      using sequence_t = sequence<numbers...>;
    };





    /** @brief calls the render method of a widget.
     *
     * @param widget the widget whose render method is to be called.
     * @param context the context to render the widget with.
     * @param tuple the remaining arguments of the render method.
     * @param - type deduction argument. ignored.
     */
    template <class return_t, class widget_t,class context_t, class tuple_t, int ... numbers>
    return_t call_render(widget_t& widget, context_t & context, const tuple_t& tuple, const sequence< numbers ... >& ){
      return widget.render(context,std::get<numbers>(tuple)... );
    }





    /** @brief fill a tuple with the values refered to by a tuple of selectors.
     *
     * @tparam i index of the current field to be filled
     * @param args_tuple tuple of the arguments later to be supplied to the render method.
     * @param selector_tuple tuple of the selectors, selecting the arguments.
     */
    template <std::size_t i, class args_tuple_t, class selector_tuple_t >
    void fill_args_tuple(args_tuple_t& args_tuple, const selector_tuple_t& selector_tuple, State& state){
      if constexpr (i < std::tuple_size_v<args_tuple_t>){
	// TODO: communication via nullptr is forbidden
	const std::tuple_element_t< i, args_tuple_t>* p =  state.get_state_ptr(std::get<i>(selector_tuple));
	if (p){
	  std::get<i>(args_tuple) = *p;
	} else {
	  assert(false && "no value from this selector");
	}


	// fill the next field of the tuple
	fill_args_tuple<i+1>(args_tuple, selector_tuple, state);
      } else{
	return;
      }

    }
  }





  /** @brief visitor to render a widget.
   *
   */
  class RenderVisitor{
  private:
    State * state_;
    RenderTree* tree_;
    CollectingContext* collecting_context_;
  public:
    RenderVisitor(
		  State* state,
		  RenderTree* tree,
		  CollectingContext* collecting_context
		  )
      :state_(state),
       tree_(tree),
       collecting_context_(collecting_context)
       {}



    /** @brief set up all arguments for the widget.render function and call it.
     *
     */
    template< class widget_t>
    void render(widget_t& widget) {

      constexpr WidgetType widget_type = widget_t::get_widget_type();
      using return_t = typename widget_t::return_t;



      Context<widget_type> context =
	Context<widget_type>(this->collecting_context_->instance_id,
			     this->tree_,
			     this->state_);


      // set up a tuple with the arguments for the widget render method
      using args_tuple_t = typename widget_t::args_tuple_t;
      args_tuple_t args_tuple{};
      detail::fill_args_tuple<0>(args_tuple, widget.selectors(), *this->state_);


      // apply the arguments to the render method
      using sequence_t =  typename detail::sequence_generator<std::tuple_size_v<args_tuple_t > >::sequence_t;
      this->collecting_context_->return_value =
	std::make_unique<ValueHolder<return_t>>(
						detail::call_render<return_t>(widget , context, args_tuple,  sequence_t() )

						);// this actually renders the widget

      // inform the context that the rendering has finished.
      context->finalize_render();
      return;

      static_assert(!std::is_same_v<return_t, void>, "a widgets return type may not be void" );
      static_assert(std::is_default_constructible_v<return_t>, "a widgets return type must be default constructible");
      static_assert(std::is_copy_constructible_v<return_t>,
		    "a widgets return type must bet copy constructible");

    }



  };

}
#endif //FURENDER_RENDER_VISITOR_HPP
