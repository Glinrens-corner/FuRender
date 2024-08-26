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
      std::cout << new_int<< std::endl;
    }
  
    NoCopyReducer( const NoCopyReducer& ) = delete;
    NoCopyReducer(  NoCopyReducer&& ) = default;
    NoCopyReducer& operator = (const NoCopyReducer & ) = delete;
    NoCopyReducer& operator = (NoCopyReducer && ) = default;
  };

  int i=0;
  auto  t = create_state_slice([]()->int{return 1;})
    .with_data_reducer<int>(NoCopyReducer(&i))
    .make_shared_ptr();

  
  std::shared_ptr<StateSlice> ss_ptr = t;
  DataEvent<int> event{};
  event.data=4;
  EventEnvelope envelope{};
  envelope.event = &event;
  
  StateDispatchVisitor visitor(envelope);
  ss_ptr->accept(visitor);
  CHECK(4 == i);
}
