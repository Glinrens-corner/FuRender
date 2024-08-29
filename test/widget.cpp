#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>

#include "basic.hpp"
#include "context_base.hpp"
#include "selector.hpp"
#include "widget.hpp"



TEST_CASE("create Widget", "[widget]"){
  using namespace fluxpp;

  auto widget1 =
    fluxpp::create_widget_with_selectors< WidgetType::Application >(Selector<int>("state/main_display"))
    .with_render_function([](Context<WidgetType::Application> &context, const int & ndisplay)->int{
      std::cout << "hello World" << std::endl;
      return 1;})
    .make_shared();

    std::shared_ptr<Widget<WidgetType::Application, int >> widget_ptr2 (widget1);
    std::shared_ptr< DeferredWidget<WidgetType::Application>  > widget2 = widget1;
}
