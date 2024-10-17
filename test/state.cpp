#include <catch2/catch_test_macros.hpp>


#include <memory>

#include "context_base.hpp"
#include "event.hpp"
#include "state.hpp"
#include "widget.hpp"



TEST_CASE("test State", "[State]"){
  using namespace furender;

  // spies to deteckt how often the state_slice/ widget_render function are executed.
  int slice_spy=0;
  int widget_spy = 0;

  // path were the StateSlice is located in the State.
  const std::string path = "path_to_state";


  // we will save a  pointer  to the StateSlice to later assert properties of it.
  TypedStateSlice<int> * slice_ptr;

  // create a slice ()
  auto slice_uptr = create_state_slice(static_cast<int>(1)/* initial value and type of the state*/)
    // Set a handler for DataEvent<int>
    //DataEvent<T> for a State containing an U requires a handler  signature of void(StateContext<U>& , const T& );
    // in production code the handler shouldn't capture anything by reference.
    // it has to be pureish.
    .with_data_reducer<int>([&slice_spy](StateContext<int>& context, const int& new_int){
      slice_spy++;
      context.update_state(new_int);
    })
    .make_unique_ptr();

  slice_ptr = slice_uptr.get();




  auto  app = create_widget_with_selectors<WidgetType::Application>()
    .with_render_function([&widget_spy](Context<WidgetType::Application>&){
      widget_spy++;
      return None::none;
    })
    .make_shared();

  DataEvent<int> event{};
  event.data=4;
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
    // state.debug_get_slices() is only for testing/debugging accessible
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


TEST_CASE("test ReducingStateSlice with a move-only reducer", "[StateSlice]"){
  using namespace furender;

  class NoCopyReducer{
  private:
    int* outer_;
  public:
    NoCopyReducer(int* outer):outer_(outer){};

    void operator()(furender::StateContext<int>& context, const int &new_int){
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
