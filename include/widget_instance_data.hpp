#ifndef FURENDER_WIDGET_INSTANCE_DATA_HPP
#define FURENDER_WIDGET_INSTANCE_DATA_HPP
#include "id_types.hpp"
#include "value_holder.hpp"
#include "widget_fwd.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace furender {

  /** @brief struct that holds all information of a given widget instance
   *
   */
  struct WidgetInstanceData {
    public:
      widget_instance_id_t instance_id;
      client_instance_id_t client_id;
      std::shared_ptr<BaseWidget> widget;
      widget_instance_id_t parent_id;
      explicit_key_t key;
      std::vector<std::pair<explicit_key_t, widget_instance_id_t>>
        children{}; // mapping (widget of subinstance, occurrence of that
                    // widget) -> instance
      std::unique_ptr<ValueHolderBase> return_value{};
  };

} // namespace furender

#endif // FURENDER_WIDGET_INSTANCE_DATA_HPP
