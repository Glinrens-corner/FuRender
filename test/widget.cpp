#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>

#include "basic.hpp"
#include "context_base.hpp"
#include "selector.hpp"
#include "widget.hpp"



TEST_CASE("create Widget", "[widget]"){
  using namespace fluxpp;

  auto widget_impl =
    fluxpp::create_widget_with_selectors< WidgetType::Application >(Selector<int>("state/main_display"))
    .with_render_function([](Context<WidgetType::Application> &context, const int & ndisplay)->int{
      return 1;})
    .make_shared();

  std::shared_ptr<Widget<WidgetType::Application, int >> widget =widget_impl;
  std::shared_ptr< DeferredWidget<WidgetType::Application>  > deferred_widget = widget_impl;
  std::shared_ptr<BaseWidget > base_widget = widget_impl;
  
}
