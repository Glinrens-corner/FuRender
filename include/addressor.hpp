#ifndef FURENDER_ADDRESSOR_HPP
#define FURENDER_ADDRESSOR_HPP

#include "selector.hpp"

#include <string>

namespace furender {
  /** @brief Class to record state, accepted events and path of a state slice
   *
   */
  template <class state_t_, class... event_ts_>
  class Addressor {
    private:
      std::string path_;

    public:
      Addressor(std::string path): path_(std::move(path)) {}

      /** @brief create a selector for the associated StateSlice
       *
       */
      Selector<state_t_> create_selector() const { return {this->path_}; }

      const std::string& path() const { return this->path_; };
  };

} // namespace furender

#endif // FURENDER_ADDRESSOR_HPP
