#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>

#include "basic.hpp"
#include "context.hpp"
#include "render_tree.hpp"
#include "render_visitor.hpp"
#include "state.hpp"
#include "widget.hpp"

TEST_CASE("simple render ()", "[render][State][Widget][RenderTree]"){
  using namespace fluxpp;
  int spy = 0;
  
  auto widget = create_widget_with_selectors<fluxpp::WidgetType::Application>()
    .with_render_function([&spy ](Context<WidgetType::Application>&){
      spy=4;
      return None::none;
    })
    .make_shared();
  State state{};
  RenderTree tree(widget, &state);
  CHECK(spy==0);
  tree.do_render();
  CHECK(spy==4);
}


TEST_CASE("simple render of a widget referencing state", "[render][State][Widget][RenderTree][StateSlice]"){
  using namespace fluxpp;
  int spy = 0;
  auto slice = create_state_slice<int>(4)
    .with_data_reducer<int>([](fluxpp::StateContext<int>& context, const int &new_int){
      context.update_state(new_int);
    }).make_unique_ptr();

  auto addressor = slice->create_addressor("here");
  
  auto widget = create_widget_with_selectors<fluxpp::WidgetType::Application>( addressor.create_selector())
    .with_render_function([&spy ](Context<WidgetType::Application>&, const int here_value){
      spy=here_value;
      return None::none;
    })
    .make_shared();
  
  
  State state{};
  state.set_state_slice(addressor,std::move(slice));
  RenderTree tree(widget, &state);
  CHECK(spy==0);
  tree.do_render();
  CHECK(spy==4);
}



TEST_CASE("render in render ()", "[render,State,Widget,RenderVisitor]"){
  using namespace fluxpp;
  int spy1 = 0;
  int spy2 = 100;
  

  auto client_widget = create_widget_with_selectors<WidgetType::Client>()
    .with_render_function([&spy2](Context<WidgetType::Client>&){
      spy2 +=1;
      return None::none;
    }).make_shared();
    
  auto widget = create_widget_with_selectors<WidgetType::Application>( )
    .with_render_function([&spy1,client_widget ](Context<WidgetType::Application>& context){

      None n = context.immediate_render(static_cast<std::shared_ptr<Widget<WidgetType::Client, None>>>(client_widget));
      (void) n;
      spy1 +=1;
      return None::none;
    })
    .make_shared();
  
  
  State state{};
  RenderTree tree(widget, &state );
  CHECK(spy1==0);
  CHECK(spy2==100);
  tree.do_render();
  CHECK(spy1==1);
  CHECK(spy2==101);
  SECTION("fluxpp tracks if a widget has to be updated"){
    
    tree.do_render();
    CHECK(spy1==1);
    CHECK(spy2==101);

    
  };
}
