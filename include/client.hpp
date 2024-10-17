#ifndef FURENDER_CLIENT_HPP
#define FURENDER_CLIENT_HPP

#include "id_types.hpp"
#include "value_holder.hpp"

#include <vector>

namespace furender {

  class Command {
    public:
      command_type_id_t command_type;
      command_instance_id_t command_instance;
      std::unique_ptr<ValueHolderBase> value_transferer;

    public:
      Command(command_type_id_t command_type,
        std::unique_ptr<ValueHolderBase> value_transferer)
        : command_type(command_type),
          value_transferer(std::move(value_transferer)) {}
  };

  struct ClientInfo {
      backend_instance_id_t backend_id;
      client_instance_id_t client_id;
      std::vector<std::string> features;
  };

  class Client {
    public:
      virtual void dispatch_commands(std::vector<Command>);
      virtual void paint();
  };

} // namespace furender

#endif // FURENDER_CLIENT_HPP
