#ifndef FLUXPP_EVENT_HPP
#define FLUXPP_EVENT_HPP

#include "basic.hpp"

#include <memory>

namespace fluxpp{
  class BaseEvent{
    virtual void get_type(){};
  };

  template <class data_t_>
  class DataEvent :
    public BaseEvent{
  public:
    data_t_ data;
  };
  
  class SignalEvent :
    public BaseEvent{
    
    
  };

  
  class EventEnvelope{
  public:
    BaseEvent * event;

  public:
    
  };
  
}


#endif // FLUXPP_EVENT_HPP
