#ifndef FURENDER_COLLECTING_CONTEXT_HPP
#define FURENDER_COLLECTING_CONTEXT_HPP

#include "client.hpp"
#include "id_types.hpp"
#include "widget_instance_data.hpp"

#include <limits>
#include <memory>
#include <sys/types.h>

namespace furender {
  struct CollectingContext;

  /** @brief subset of data in CollectionContext that is needed to create a new
   * WidgetInstanceData struct.
   *
   */
  struct WidgetInstanceCreationData {
      // widget is already set in the surrounding context
      std::vector<std::pair<explicit_key_t, widget_instance_id_t>> children{};
      std::vector<std::pair<explicit_key_t, std::unique_ptr<CollectingContext>>>
        subcontexts{};
      std::unique_ptr<ValueHolderBase> return_value{};
  };

  /**@brief collecting data to later render the widget_instances
   *
   */

  struct CollectingContext {
    public:
      client_instance_id_t
        client_id{}; // the Client to which to dispatch the commands.
      widget_instance_id_t parent_id{}; // id of the parent widget
      explicit_key_t key =
        std::numeric_limits<explicit_key_t>::max(); // key to find this widget
                                                    // in the parents children
      // for debugging set it to max before using it.
      // (max is a valid, but unlikely value)
      widget_instance_id_t instance_id{}; // id of this widget
      std::shared_ptr<BaseWidget> widget = nullptr;
      std::vector<std::unique_ptr<Command>> commands{};
      // commands for the client
      std::optional<WidgetInstanceData *> old_instance_data{};
      // data of the previous rendering of the instance.
      std::optional<WidgetInstanceCreationData> new_instance_data{};
      // if the renderer doesn't set new instance data,
      // we should reuse the old_instance.
      // in this case old_instance_data must be set
  };

} // namespace furender

#endif // FURENDER_COLLECTING_CONTEXT_HPP
