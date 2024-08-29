
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
#include "render_tree.hpp"
#include "state.hpp"


namespace fluxpp {
  
  namespace detail{
    template <int... >
    struct seq{};

    template<int N, int ...S>
    struct gen: gen <N-1,N-1,S...>{};

    template <int ... S>
    struct gen<0, S...> {
      using sequence_t = seq<S...>;
    };

    template <class widget_t,class context_t, class tuple_t, int ... S>
    void apply_tuple(widget_t& widget, context_t & context, const tuple_t& tuple, const seq<S...>& ){
      widget.render(context,std::get<S>(tuple)... );
    }
    
    template <std::size_t i, class tuple_t, class selector_tuple_t >
    void fill_args_tuple(tuple_t& tuple, const selector_tuple_t& selector_tuple, State& state){
      if constexpr (i < std::tuple_size_v<tuple_t>){
	const std::tuple_element_t< i, tuple_t>* p =  state.get_state_ptr(std::get<i>(selector_tuple));
	if (p){
	  std::cout<< " get_state ptr returned:" << *p << std::endl; 	  
	  std::get<i>(tuple) = *p;
	} else {
	  std::cout<< " get_state ptr returned a nullptr;" << std::endl; 
	}
	 
	fill_args_tuple<i+1>(tuple, selector_tuple, state);
      } else{
	return;
      }
      
    }
  }





  
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


    
    template< class widget_t>
    void render(widget_t& widget){
      
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
      Context<widget_type_> context(this->current_node_, this->tree_);
      using args_tuple_t = typename widget_t::args_tuple_t;

      args_tuple_t args_tuple{};
      detail::fill_args_tuple<0>(args_tuple, widget.selectors(), *this->state_);
      using sequence_t =  typename detail::gen<std::tuple_size_v<args_tuple_t > >::sequence_t;
      detail::apply_tuple(widget , context, args_tuple,  sequence_t() );
      return;

    };
  };

}
#endif //FLUXPP_RENDER_VISITOR_HPP
