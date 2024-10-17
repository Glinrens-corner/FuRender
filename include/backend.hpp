#ifndef FURENDER_BACKEND_HPP
#define FURENDER_BACKEND_HPP

#include <memory>
#include <vector>

#include "value_holder.hpp"
#include "id_types.hpp"
#include "client.hpp"


namespace furender{




  class Backend{
  public:
    virtual void ensure_initialized(backend_instance_id_t);
    virtual Client* retrieve_client_ptr(client_instance_id_t);
    virtual  ~Backend();
  };
}



#endif
