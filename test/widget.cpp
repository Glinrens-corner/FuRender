#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "basic.hpp"
#include "widget.hpp"






TEST_CASE("create Widget", "[widget]"){
  using namespace fluxpp;

  
    auto widget1 =
    create_widget_with_selectors< WidgetType::Application>(Selector<int>("state/main_display"))
    .with_render_function([](Context<WidgetType::Application> &context, const int & ndisplay){
      std::cout << "hello World" << std::endl;})
    .make_shared();

    std::shared_ptr<Widget<WidgetType::Application, void>> widget_ptr2 (widget1);
    std::shared_ptr<ErasedWidget<WidgetType::Application>> widget2 = widget1;
}
