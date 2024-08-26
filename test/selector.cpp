#include <type_traits>
#include <catch2/catch_test_macros.hpp>

#include "selector.hpp"





TEST_CASE("Selector", "[selector]"){
  using namespace fluxpp;
  auto S = Selector<int>("asdf");
  CHECK(std::is_same<typename decltype(S)::selected_t, int>::value);
  CHECK(S.path() == "asdf");
}

