#ifndef FURENDER_UI_HPP
#define FURENDER_UI_HPP

#include "backend.hpp"
#include "render_tree.hpp"
#include "state.hpp"

#include <memory>

namespace furender {

  class UI {
    private:
      std::unique_ptr<State> state_;
      std::unique_ptr<RenderTree> render_tree_;
      std::vector<std::unique_ptr<Backend>> backends_;

    public:
      UI(std::shared_ptr<DeferredWidget<WidgetType::Application>> root_widget,
        std::unique_ptr<State> state,
        std::vector<std::unique_ptr<Backend>> backends)
        : state_(std::move(state)),
          backends_(std::move(backends)) {
        this->render_tree_ = std::make_unique<RenderTree>(
          std::move(root_widget), this->state_.get());
        this->state_->set_render_tree(this->render_tree_.get());
      };

      void start();
  };

} // namespace furender

#endif // FURENDER_UI_HPP
