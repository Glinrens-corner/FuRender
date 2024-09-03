#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <memory>

#include "context_base.hpp"
#include "event.hpp"
#include "state.hpp"
#include "widget.hpp"


TEST_CASE("test ReducingStateSlice with a move-only reducer", "[StateSlice]"){
  using namespace fluxpp;

  class NoCopyReducer{
  private:
    int* outer_;
  public:
    NoCopyReducer(int* outer):outer_(outer){};

    void operator()(fluxpp::StateContext<int>& context, const int &new_int){
      *outer_ = new_int;
      context.update_state(new_int);
    }

    NoCopyReducer( const NoCopyReducer& ) = delete;
    NoCopyReducer(  NoCopyReducer&& ) = default;
    NoCopyReducer& operator = (const NoCopyReducer & ) = delete;
    NoCopyReducer& operator = (NoCopyReducer && ) = default;
  };

  int i=0;
  auto u_ptr = create_state_slice(static_cast<int>(1))
    .with_data_reducer<int>(NoCopyReducer(&i))
    .make_unique_ptr();


  DataEvent<int> event{};
  event.data=4;
  EventEnvelope envelope{};
  envelope.event = &event;
  envelope.path = "path_to_state";
  StateDispatchVisitor visitor(envelope);
  bool updated = u_ptr->accept_dispatch_visitor(visitor);
  CHECK(updated);
  CHECK(u_ptr->state() == 4);
  CHECK(4 == i);
}


TEST_CASE("test State", "[State]"){
  using namespace fluxpp;


  int slice_spy=0;
  auto slice_uptr = create_state_slice(static_cast<int>(1))
    .with_data_reducer<int>([&slice_spy](StateContext<int>& context, const int& new_int){
      slice_spy++;
      context.update_state(new_int);
    })
    .make_unique_ptr();
  auto slice_ptr = slice_uptr.get();

  int widget_spy = 0;
  auto  app = create_widget_with_selectors<WidgetType::Application>()
    .with_render_function([&widget_spy](Context<WidgetType::Application>&){
      widget_spy++;
      return None::none;
    })
    .make_shared();

  DataEvent<int> event{};
  event.data=4;
  std::string path = "path_to_state";
  EventEnvelope envelope{};
  envelope.event = &event;
  envelope.path = path;
  State state{};
  // state requires! a tree to be set.
  RenderTree tree(app, &state);
  state.set_render_tree(&tree);
  // State is now initialized, set slice(s)
  state.set_state_slice(path, std::move(slice_uptr));

  {
    // state.slices_ is only for testing/debugging accessible
    REQUIRE(state.debug_get_slices().size() == 1);
    auto& slice_data =   state.debug_get_slices().at(path);
    REQUIRE(slice_data.subscriptions.size() == 0);
    REQUIRE(slice_data.slice.get() == slice_ptr);
  }
  state.dispatch_event(envelope);
  {
    REQUIRE(state.debug_get_slices().size() == 1);
    auto& slice_data =   state.debug_get_slices().at(path);
    CHECK(slice_data.subscriptions.size() == 0);
    REQUIRE(slice_data.slice.get() == slice_ptr);
  }
  CHECK(slice_ptr->state() == 4);
  CHECK( slice_spy == 1);
}
