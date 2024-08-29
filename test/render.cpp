#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "basic.hpp"
#include "context_base.hpp"
#include "render_tree.hpp"
#include "render_visitor.hpp"
#include "state.hpp"
#include "widget.hpp"

TEST_CASE("simple render ()", "[render,State,Widget,RenderVisitor]"){
  using namespace fluxpp;
  int i = 0;
  
  auto widget = create_widget_with_selectors<fluxpp::WidgetType::Application>()
    .with_render_function([&i ](Context<WidgetType::Application>&){
      i=4;
      return None::none;
    })
    .make_shared();
  State state{};
  RenderNode node{};
  node.widget = widget;
  RenderVisitor visitor(&state, &node, nullptr) ;
  CHECK(i==0);
  widget->accept(visitor);
  CHECK(i==4);
}


TEST_CASE("simple render ( int )", "[render,State,Widget,RenderVisitor]"){
  using namespace fluxpp;
  int i = 0;
  
  auto widget = create_widget_with_selectors<fluxpp::WidgetType::Application>( Selector<int>("here"))
    .with_render_function([&i ](Context<WidgetType::Application>&, const int here_value){
      std::cout << "updating value from:" << i << " to:" << here_value<< std::endl; 
      i=here_value;
      return None::none;
    })
    .make_shared();
  
  auto slice = create_state_slice([](){
    std::cout << "initialized " << std::endl;
    return 4;})
    .with_data_reducer<int>([](fluxpp::StateContext<int>& context, const int &new_int){
      context.update_state(new_int);
    }).make_unique_ptr();
  
  
  State state{};
  state.set_state_slice("here",std::move(slice));
  RenderNode node{};
  node.widget = widget;
  RenderVisitor visitor(&state, &node, nullptr) ;
  CHECK(i==0);
  widget->accept(visitor);
  CHECK(i==4);
}
