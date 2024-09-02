#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>

#include "basic.hpp"
#include "context.hpp"
#include "id_types.hpp"
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
  REQUIRE(spy==0);
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
    .with_render_function([&spy ](Context<WidgetType::Application>&, const int new_value){
      spy=new_value;
      return None::none;
    })
    .make_shared();
  
  
  State state{};
  RenderTree tree(widget, &state);
  state.set_render_tree(&tree);

  state.set_state_slice(addressor,std::move(slice));
  REQUIRE(spy==0);
  {
    REQUIRE(state.slices_.size() == 1  );
    auto & slice_data = state.slices_.at(addressor.path());
    REQUIRE(slice_data.subscriptions.size() == 0);
  }
  tree.do_render();
  CHECK(spy==4);
}



TEST_CASE("render without update", "[render][State][Widget][RenderTree]"){
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



TEST_CASE("render with update", "[render][State][Widget][RenderTree][StateSlice]"){
  using namespace fluxpp;
  int spy1 = 0;
  int spy2 = 100;

  auto slice = create_state_slice<int>(1)
    .with_data_reducer<int>([](fluxpp::StateContext<int>& context, const int &new_int){
      context.update_state(new_int);
    }).make_unique_ptr();

  auto addressor = slice->create_addressor("slice_path");
  
  auto client_widget = create_widget_with_selectors<WidgetType::Client>()
    .with_render_function([&spy2](Context<WidgetType::Client>&){
      spy2 +=1;
      return None::none;
    }).make_shared();
    
  auto app_widget = create_widget_with_selectors<WidgetType::Application>( addressor.create_selector())
    .with_render_function([&spy1,client_widget ](Context<WidgetType::Application>& context, const int& other_value){

      None n = context.immediate_render(static_cast<std::shared_ptr<Widget<WidgetType::Client, None>>>(client_widget));
      (void) n;
      spy1 +=1;
      return None::none;
    })
    .make_shared();

  
  State state{};
  state.set_state_slice(addressor, std::move(slice));
  RenderTree tree(app_widget, &state );
  state.set_render_tree(&tree);
  REQUIRE(spy1==0);
  REQUIRE(spy2==100);
  tree.do_render();
  CHECK(spy1==1);
  CHECK(spy2==101);
  SECTION("if the state_slice hasn't been updated, the next render doesn't render anything"){
    tree.do_render();
    CHECK(spy1==1);
    CHECK(spy2==101);
    
  };

  SECTION("if the state_slice has updated, the next render rerenders the affected widget"){

    {
      CHECK(tree.render_tree_.size() == 2);
      CHECK(tree.root_instance_ != widget_null_instance);
      CHECK(tree.widget_instances_to_update_.size() == 0);
    }
    {
      auto& slice_data = state.slices_[addressor.path()];
      CHECK(slice_data.subscriptions.size() == 1);
    }
    DataEvent<int> event{};
    event.data=4;
    EventEnvelope envelope{};
    envelope.event = &event;
    envelope.path = addressor.path();
    state.dispatch_event(envelope);
    {
      CHECK(tree.render_tree_.size() == 2);
      CHECK(tree.root_instance_ != widget_null_instance);
      CHECK(tree.widget_instances_to_update_.size() == 1);
    }

    tree.do_render();
    {
      //      CHECK(tree.render_tree_.size() == 2);
      CHECK(tree.root_instance_ != widget_null_instance);
      CHECK(tree.widget_instances_to_update_.size() == 0);
    }
    CHECK(spy1==2);
    
  };
  
}
