
#ifndef FLUXPP_RENDER_VISITOR_HPP
#define FLUXPP_RENDER_VISITOR_HPP

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include <iostream>

#include "basic.hpp"
#include "context.hpp"

#include "value_holder.hpp"

#include "render_tree_fwd.hpp"

#include "state.hpp"

namespace fluxpp {
  
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





    /** @brief apply a tuple of arguments to the render function of a widget.
     *
     */
    template <class widget_t,class context_t, class tuple_t, int ... numbers>
    void apply_tuple(widget_t& widget, context_t & context, const tuple_t& tuple, const sequence< numbers ... >& ){
      widget.render(context,std::get<numbers>(tuple)... );
    }





    /** @brief fill a tuple with the values refered to by a tuple of selectors.
     *
     */
    template <std::size_t i, class args_tuple_t, class selector_tuple_t >
    void fill_args_tuple(args_tuple_t& args_tuple, const selector_tuple_t& selector_tuple, State& state){
      if constexpr (i < std::tuple_size_v<args_tuple_t>){
	const std::tuple_element_t< i, args_tuple_t>* p =  state.get_state_ptr(std::get<i>(selector_tuple));
	if (p){
	  std::get<i>(args_tuple) = *p;
	} else {
	}
	 
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
    RenderNode* current_node_;
    RenderTree* tree_;
  public:
    RenderVisitor(State* state, RenderNode* node, RenderTree* tree)
      :state_(state),
       current_node_(node),
       tree_(tree){}



    /** @brief set up all arguments for the widget.render function
     *
     */
    template< class widget_t>
    void render(widget_t& widget) const{
      constexpr WidgetType widget_type_ = widget_t::get_widget_type();
      using return_t = typename widget_t::return_t;
      static_assert(!std::is_same_v<return_t, void>, "a widgets return type may not be void" );
      static_assert(std::is_default_constructible_v<return_t>, "a widgets return type must be default constructible");
      static_assert(std::is_copy_constructible_v<return_t>,
		    "a widgets return type must bet copy constructible");

      ValueHolder<return_t>* p = nullptr;
      if(this->current_node_->return_value ){
	p = dynamic_cast<ValueHolder<return_t>* >(this->current_node_->return_value.get()) ; 
	if(!p){
	  // ERROR return type isn't identical to the previous one.
	  p = new ValueHolder<return_t>(return_t() );
	  this->current_node_->return_value = std::unique_ptr<ValueHolderBase>(p);
	}
      } else {
	  p = new ValueHolder<return_t>(return_t() );
	  this->current_node_->return_value = std::unique_ptr<ValueHolderBase>(p);
      }
      Context<widget_type_> context(this->current_node_, this->tree_, this->state_);
      using args_tuple_t = typename widget_t::args_tuple_t;

      args_tuple_t args_tuple{};
      detail::fill_args_tuple<0>(args_tuple, widget.selectors(), *this->state_);
      using sequence_t =  typename detail::sequence_generator<std::tuple_size_v<args_tuple_t > >::sequence_t;
      detail::apply_tuple(widget , context, args_tuple,  sequence_t() );
      return;
    };
  };

}
#endif //FLUXPP_RENDER_VISITOR_HPP
