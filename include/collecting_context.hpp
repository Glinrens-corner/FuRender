#ifndef FURENDER_COLLECTING_CONTEXT_HPP
#define FURENDER_COLLECTING_CONTEXT_HPP



#include <memory>

#include "client.hpp"
#include "id_types.hpp"
#include "widget_instance_data.hpp"

namespace furender{

  struct CollectingContext{
  public:
    std::optional<client_instance_id_t> context_client;
    bool is_old_data_current = false;
    widget_instance_id_t parent_id;
    widget_instance_id_t instance_id;
    std::shared_ptr<BaseWidget> widget;
    std::unique_ptr<ValueHolderBase> return_value{};
    std::vector<std::unique_ptr<Command> > commands{} ;
    std::optional<WidgetInstanceData*> old_instance_data;
    std::vector<std::pair<explicit_key_t,
			  std::unique_ptr<CollectingContext> > >subcontexts{};
  public:

  };

}

#endif //FURENDER_COLLECTING_CONTEXT_HPP
