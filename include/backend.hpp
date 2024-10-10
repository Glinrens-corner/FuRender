#ifndef FURENDER_BACKEND_HPP
#define FURENDER_BACKEND_HPP

#include <memory>

#include "value_holder.hpp"
#include "id_types.hpp"


namespace furender{


  class Command{
  public:
    command_type_id command_type;
    std::unique_ptr<ValueHolderBase> value_transferer;
  public:
    Command(command_type_id command_type, std::unique_ptr<ValueHolderBase>  value_transferer):
      command_type(command_type),
      value_transferer(std::move(value_transferer)){}



  };


  class Backend{
  public:
    virtual command_type_id get_command_id(std::string_view command_name);

  };
}



#endif
