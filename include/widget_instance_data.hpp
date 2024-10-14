#ifndef FURENDER_WIDGET_INSTANCE_DATA_HPP
#define FURENDER_WIDGET_INSTANCE_DATA_HPP
#include <memory>
#include <optional>
#include <vector>

#include "id_types.hpp"
#include "value_holder.hpp"
#include "widget_fwd.hpp"

namespace furender{

  /** @brief struct that holds all information of a given widget instance
   *
   */
  struct WidgetInstanceData{
  public:
    std::optional<client_instance_id_t> context_client;
    std::shared_ptr<BaseWidget> widget;
    widget_instance_id_t parent;
    std::vector<std::pair<explicit_key_t,widget_instance_id_t> >
    children{};  // mapping (widget of subinstance, occurrence of that widget) -> instance
    std::unique_ptr<ValueHolderBase> return_value{};
  };



}


#endif //FURENDER_WIDGET_INSTANCE_DATA_HPP
