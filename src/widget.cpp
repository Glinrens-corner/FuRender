
#include "widget.hpp"
#include "id_types.hpp"
#include <mutex>

static std::mutex widget_id_mutex{};

furender::widget_id_t furender::detail::create_widget_id(){
  const std::lock_guard<std::mutex> lock(widget_id_mutex);
  static uint64_t i=0;
  return widget_id_t(i++);
}
