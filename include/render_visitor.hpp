
#ifndef FLUXPP_RENDER_VISITOR_HPP
#define FLUXPP_RENDER_VISITOR_HPP

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

#include "id_types.hpp"
#include "value_holder.hpp"

#include "render_tree.hpp"

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
    template <class return_t, class widget_t,class context_t, class tuple_t, int ... numbers>
    return_t apply_tuple(widget_t& widget, context_t & context, const tuple_t& tuple, const sequence< numbers ... >& ){
      return widget.render(context,std::get<numbers>(tuple)... );
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
    widget_instance_id_t instance_id_;
    std::shared_ptr<BaseWidget> widget_;
    widget_instance_id_t parent_id_; 
    RenderTree* tree_;
    std::optional<RenderNode> node_opt_{};
  public:
    RenderVisitor(
		  State* state,
		  widget_instance_id_t instance_id,
		  std::shared_ptr<BaseWidget> widget,
		  widget_instance_id_t parent_id,
		  RenderTree* tree)
      :state_(state),
       instance_id_(instance_id),
       widget_(std::move(widget)),
       parent_id_(parent_id),
       tree_(tree){}



    /** @brief set up all arguments for the widget.render function
     *
     */
    template< class widget_t>
    void render(widget_t& widget) {
      constexpr WidgetType widget_type_ = widget_t::get_widget_type();
      using return_t = typename widget_t::return_t;
      RenderNode* node = this->get_node_ptr();
      
      static_assert(!std::is_same_v<return_t, void>, "a widgets return type may not be void" );
      static_assert(std::is_default_constructible_v<return_t>, "a widgets return type must be default constructible");
      static_assert(std::is_copy_constructible_v<return_t>,
		    "a widgets return type must bet copy constructible");

      Context<widget_type_> context(this->instance_id_,node, this->tree_, this->state_);
      
      using args_tuple_t = typename widget_t::args_tuple_t;
      args_tuple_t args_tuple{};

      detail::fill_args_tuple<0>(args_tuple, widget.selectors(), *this->state_);

      using sequence_t =  typename detail::sequence_generator<std::tuple_size_v<args_tuple_t > >::sequence_t;

      ValueHolder<return_t>* p = nullptr;
      if(node->return_value ){
	p = dynamic_cast<ValueHolder<return_t>* >(node->return_value.get()) ; 
	if(!p){
	  // ERROR return type isn't identical to the previous one.
          node->return_value = std::unique_ptr<ValueHolderBase>(
								new ValueHolder<return_t>(
											       detail::apply_tuple<return_t>(widget , context, args_tuple,  sequence_t() )         )
								);
	}else {
	  p->data = detail::apply_tuple<return_t>(widget , context, args_tuple,  sequence_t() );
	}
      } else {
            node->return_value = std::unique_ptr<ValueHolderBase>(
								  new ValueHolder<return_t>(
											    detail::apply_tuple<return_t>(widget , context, args_tuple,  sequence_t() )
											    )
                );
      }
      this->store_render_node();
      this->tree_->rendered_instance(this->instance_id_);
      return;
    }

  private:
    RenderNode* get_node_ptr();

    void store_render_node();
  };

}
#endif //FLUXPP_RENDER_VISITOR_HPP
