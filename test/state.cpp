#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <memory>

#include "event.hpp"
#include "state.hpp"

  
TEST_CASE("test ReducingStateSlice", "[StateSlice]"){
  using namespace fluxpp;
  
  class NoCopyReducer{
  private:
    int* outer_;
  public:
    NoCopyReducer(int* outer):outer_(outer){};
    
    void operator()(fluxpp::StateContext<int>& context, const int &new_int){
      *outer_ = new_int;
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
  
  StateDispatchVisitor visitor(envelope);
  u_ptr->accept_dispatch_visitor(visitor);
  CHECK(4 == i);
}
