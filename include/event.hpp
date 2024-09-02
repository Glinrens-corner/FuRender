#ifndef FLUXPP_EVENT_HPP
#define FLUXPP_EVENT_HPP

#include <memory>

#include "basic.hpp"


namespace fluxpp{

  /** @brief base class for all event types
   *
   */
  class BaseEvent{
    // some virtual function so BaseEvent is polymorphic.
    virtual void get_type(){};
  };





  /** @brief an event type to send data of any specific type through the event system.
   * 
   */
  template <class data_t_>
  class DataEvent :
    public BaseEvent{
  public:
    data_t_ data;
  };





  /** @brief An event class to send a signal without any state through the event system.
   *
   */
  class SignalEvent :
    public BaseEvent{
    
  };


  /** @brief envelope for use in the event system.
   *
   */
  class EventEnvelope{
  public:
    BaseEvent * event;
    std::string path;
  };
  
}


#endif // FLUXPP_EVENT_HPP
