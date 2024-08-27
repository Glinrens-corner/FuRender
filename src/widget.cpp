
#include "widget.hpp"
#include <mutex>

static std::mutex widget_id_mutex{};

uint64_t fluxpp::detail::create_widget_id(){
  const std::lock_guard<std::mutex> lock(widget_id_mutex);
  static uint64_t i=0;
  return i++;
}
    

