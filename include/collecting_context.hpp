#ifndef FURENDER_COLLECTING_CONTEXT_HPP
#define FURENDER_COLLECTING_CONTEXT_HPP



#include <limits>
#include <memory>
#include <sys/types.h>

#include "client.hpp"
#include "id_types.hpp"
#include "widget_instance_data.hpp"

namespace furender{
  struct CollectingContext;
  
  struct WidgetInstanceCreationData{
    // widget is already set in the surrounding context
    std::vector<std::pair<explicit_key_t,
			  widget_instance_id_t> > children{};
    std::vector<std::pair<explicit_key_t,
			  std::unique_ptr<CollectingContext> > >subcontexts{};
    std::unique_ptr<ValueHolderBase> return_value{};
  };
  
  struct CollectingContext{
  public:
    widget_instance_id_t client_id; // the Client to which to dispatch the commands.
    widget_instance_id_t parent_id{};
    explicit_key_t key=std::numeric_limits<explicit_key_t>::max(); // key to find this widget in the parents children
    // for debugging set it to max before using it.
    // (max is a valid, but unlikely value)
    widget_instance_id_t instance_id{};
    std::shared_ptr<BaseWidget> widget=nullptr;
    std::vector<std::unique_ptr<Command> > commands{} ;
    std::optional<WidgetInstanceData*> old_instance_data{};
    std::optional<WidgetInstanceCreationData> new_instance_data{};

  };

}

#endif //FURENDER_COLLECTING_CONTEXT_HPP
