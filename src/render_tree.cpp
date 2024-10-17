
#include "render_tree.hpp"

#include "collecting_context.hpp"
#include "id_types.hpp"
#include "render_visitor.hpp"
#include "renderer.hpp"
#include "widget.hpp"
#include "widget_instance_data.hpp"

#include <assert.h>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace furender {

  RenderTree::RenderTree(
    std::shared_ptr<DeferredWidget<WidgetType::Application>> root_widget,
    State *state)
    : state_(state),
      root_widget_(std::move(root_widget)) {}

  widget_instance_id_t RenderTree::get_next_widget_to_update() const {
    auto instance_to_update_it = this->widget_instances_to_update_.begin();
    {
      widget_instance_id_t parent =
        this->render_tree_.find(*instance_to_update_it)->second.parent_id;
      while (parent != widget_null_instance) {
        widget_instance_id_t current = parent;
        auto update_it = this->widget_instances_to_update_.find(current);
        if (update_it != this->widget_instances_to_update_.end()) {
          instance_to_update_it = update_it;
        }
        parent = this->render_tree_.find(current)->second.parent_id;
      }
    }
    return *instance_to_update_it;
  }

  void RenderTree::render_all() {
    // on the first render, render the application widget.
    // since no other other instances exist yet, none can be subscribed to
    // and none can be necessary to update
    if (not this->root_instance_.has_value()) {
      assert(this->widget_instances_to_update_.empty() &&
             "there are no widgets that could already be needed to update");
      CollectingContext collection_context;
      this->render_root(&collection_context);
      while (!this->render_queue_.empty()) {
        RenderQueueEntry& entry = this->render_queue_.back();
        this->render_render_queue_entry(entry);
        this->render_queue_.pop_back();
      }
      this->merge_collecting_context(collection_context);
      // otherwise rerender all instances to create
    } else {

      while (!this->widget_instances_to_update_.empty()) {
        if (not this->root_instance_.has_value()) {
          assert(
            false && "error: no tree, but instances in the tree to update??");
          break;
        }

        CollectingContext collection_context;
        auto next_instance = this->get_next_widget_to_update();
        this->rerender_instance(next_instance, &collection_context);
        while (!this->render_queue_.empty()) {
          RenderQueueEntry& entry = this->render_queue_.back();
          this->render_render_queue_entry(entry);
          this->render_queue_.pop_back();
        }
        this->merge_collecting_context(collection_context);
      }
    }
  }

  void RenderTree::render_root(CollectingContext *collecting_context) {
    collecting_context->parent_id = widget_null_instance;

    assert(this->root_widget_);
    detail::Renderer(this, this->state_)
      .render_internal(0 /* doesn't matter*/, widget_null_instance,
        this->root_widget_, collecting_context);
    assert(collecting_context->new_instance_data.has_value() &&
           "root widget needs to be newly rendered.");
    this->root_instance_ = collecting_context->instance_id;
  }

  void RenderTree::render_render_queue_entry(RenderQueueEntry& entry) {

#ifndef NDEBUG
    if (entry.parent_id != widget_null_instance) {
      auto it = this->render_tree_.find(entry.parent_id);
      assert(
        it != this->render_tree_.end() && "parent of entry doesn't exists");
      // assert(std::find(
      // it->second.subinstances.begin(),it->second.subinstances.end(),entry.instance)
      // != it->second.subinstances.end() && "instance not subinstance of
      // parent");
    }
#endif // NDEBUG
    CollectingContext collecting_context;

    detail::Renderer(this, this->state_)
      .render_internal(entry.key, entry.parent_id, std::move(entry.widget),
        &collecting_context);
  }

  void RenderTree::rerender_instance(widget_instance_id_t new_instance_id,
    CollectingContext *collecting_context) {
    auto child_it = this->render_tree_.find(new_instance_id);

    assert(
      child_it != this->render_tree_.end() && "instance to rerender not found");
    explicit_key_t key;
    if (child_it->second.parent_id == widget_null_instance) {
      key = 0;
    } else {
      auto parent_it = this->render_tree_.find(new_instance_id);

      assert(parent_it != this->render_tree_.end() &&
             "no parent instance, but not root");
      auto mapping_it = std::find_if(parent_it->second.children.begin(),
        parent_it->second.children.end(),
        [new_instance_id](
          const std::pair<explicit_key_t, widget_instance_id_t>& pair) {
          return pair.second == new_instance_id;
        });
      assert(mapping_it != parent_it->second.children.end() &&
             "instance not a child of parent");
      key = mapping_it->first;
    }

    detail::Renderer(this, this->state_)
      .render_internal(key, child_it->second.parent_id, child_it->second.widget,
        collecting_context);
  }

  std::optional<WidgetInstanceData *> RenderTree::get_widget_instance_data(
    widget_instance_id_t id) {
    auto it = this->render_tree_.find(id);
    if (it != this->render_tree_.end()) {
      return &it->second;
    } else {
      return {};
    }
  }

  void RenderTree::empty_deletion_stack() {
    while (not this->deletion_set_.empty()) {
      widget_instance_id_t next_instance_id = *this->deletion_set_.begin();
      this->delete_instance(next_instance_id);
    }
  }

  /** @brief merges a tree of collecting contexts
   *
   *
   */
  void RenderTree::merge_collecting_context(
    CollectingContext& root_collecting_context) {
    // walking the tree of collecting_contexts in a DFS.
    std::vector<CollectingContext *> collecting_context_stack{
      &root_collecting_context};
    while (not collecting_context_stack.empty()) {
      // while stack_top has children, push a new (pointer to context) onto the
      // stack
      if (collecting_context_stack.back()->new_instance_data.has_value() &&
          (not collecting_context_stack.back()
                 ->new_instance_data.value()
                 .subcontexts.empty())) {

        collecting_context_stack.push_back(collecting_context_stack.back()
                                             ->new_instance_data.value()
                                             .subcontexts.back()
                                             .second.get());
      } else {
        CollectingContext& current_context = *collecting_context_stack.back();
        // the collecting_context will be completely handled. therfore we can
        // already erase it.
        collecting_context_stack.pop_back();

        // if there is no new instance data, the widget doesn't have to be
        // updated.
        if (!current_context.new_instance_data.has_value()) {
          // ... but the old instance data must be present.
          assert(current_context.old_instance_data.has_value() &&
                 "if the old instance has all data, it must be set. ");

          auto instance_id = current_context.instance_id;
          assert(
            this->widget_instances_to_update_.erase(instance_id) == 0 &&
            "if the old data is sufficient for the instance, it shouldn't be marked 'to update'");
          if (collecting_context_stack.size() > 0) {
            CollectingContext& parent_context =
              **(collecting_context_stack.rbegin());
            assert(
              parent_context.new_instance_data.has_value() &&
              "if there are subcontexts, the parent_context must be recreated.  and ave data for the new creation");
            // using the fact, that we always push a pointer to the last
            // subcontext onto the stack. therefore the last subcontext of
            // parent ist the current context
            auto& [key, collecting_context_uptr /*unused */] =
              parent_context.new_instance_data.value().subcontexts.back();
            assert(
              collecting_context_uptr.get() == &current_context &&
              "the accessed subcontext of the parent must be the same as the current context");

            assert(
              current_context.old_instance_data.value()->parent_id ==
                current_context.parent_id &&
              " the context parent_id and the widget_instance_data parent_id must be the same.");
            assert(current_context.parent_id == parent_context.instance_id &&
                   "the parent id must be the instance id of the parent ");

            assert(
              current_context.instance_id ==
                current_context.old_instance_data.value()->instance_id &&
              "the context and the widget_instance_data must have the same id");

            parent_context.new_instance_data.value().children.emplace_back(
              key, current_context.instance_id);
            parent_context.new_instance_data.value().subcontexts.pop_back();
          }
        } else {
          WidgetInstanceData instance_data{};
          assert((current_context.instance_id != widget_instance_id_t{}) &&
                 "a new instance_id has to have been set.");
          instance_data.instance_id = current_context.instance_id;
          instance_data.client_id = current_context.client_id;
          assert(current_context.widget && "widget must be set.");
          instance_data.widget = current_context.widget;
          instance_data.parent_id = current_context.parent_id;
          instance_data.key = current_context.key;
          // collection context is soon to be destroyed, we can move the
          // children out.
          instance_data.children =
            std::move(current_context.new_instance_data.value().children);

          assert(current_context.new_instance_data.value().return_value);
          instance_data.return_value =
            std::move(current_context.new_instance_data.value().return_value);
          this->insert_or_update_instance(
            std::move(instance_data), current_context.old_instance_data);

          if (collecting_context_stack.size() > 0) {
            CollectingContext& parent_context =
              **(collecting_context_stack.rbegin());
            assert(
              parent_context.new_instance_data.has_value() &&
              "if there are subcontexts, the parent_context must be recreated"
              "  and have data for the new creation");
            // using the fact, that we always push a pointer to the last
            // subcontext
            // onto the stack. therefore the last subcontext of parent ist the
            // current context
            auto& [key, collecting_context_uptrref /*unused */] =
              parent_context.new_instance_data.value().subcontexts.back();
            assert(
              collecting_context_uptrref.get() == &current_context &&
              "the accessed subcontext of the parent must be the same as the current context");

            if (current_context.old_instance_data.has_value()) {
              assert(
                current_context.old_instance_data.value()->parent_id ==
                  current_context.parent_id &&
                " the context parent_id and the widget_instance_data parent_id must be the same.");
            }

            assert(current_context.parent_id == parent_context.instance_id &&
                   "the parent id must be the instance id of the parent ");

            if (current_context.old_instance_data.has_value()) {
              assert(
                current_context.instance_id ==
                  current_context.old_instance_data.value()->instance_id &&
                "the context and the widget_instance_data must have the same id");
            }
            parent_context.new_instance_data.value().children.emplace_back(
              key, current_context.instance_id);
            parent_context.new_instance_data.value().subcontexts.pop_back();
          }
        }
      }
    }
  }

  void RenderTree::insert_or_update_instance(WidgetInstanceData&& new_data,
    std::optional<WidgetInstanceData *> old_data_optr) {

    if (old_data_optr.has_value()) {
      WidgetInstanceData& old_data = *old_data_optr.value();
      this->remove_subscriptions_from_state(old_data);
      for (auto [key, child_id] : old_data.children) {
        (void)key;
        this->deletion_set_.insert(child_id);
      }
      auto instance_id = old_data.instance_id;
      this->render_tree_.erase(instance_id);
    }
    this->subscribe_to_state(new_data);
    for (auto [key, child_id] : new_data.children) {
      (void)key;
      this->deletion_set_.erase(child_id);
    }
    auto instance_id = new_data.instance_id;
    this->widget_instances_to_update_.erase(instance_id);
    this->deletion_set_.erase(instance_id);
    this->render_tree_.insert({instance_id, std::move(new_data)});
  }

  void RenderTree::remove_subscriptions_from_state(
    const WidgetInstanceData& instance_data) {
    for (std::size_t ipath = 0;; ipath++) {
      std::optional<std::reference_wrapper<const std::string>> path_opt =
        instance_data.widget->get_nth_selector_address(ipath);
      if (path_opt.has_value()) {
        this->state_->remove_subscription(
          path_opt.value().get(), instance_data.instance_id);
      } else {
        break;
      }
    }
  }

  void RenderTree::subscribe_to_state(const WidgetInstanceData& instance_data) {
    for (std::size_t ipath = 0;; ipath++) {

      std::optional<std::reference_wrapper<const std::string>> path_opt =
        instance_data.widget->get_nth_selector_address(ipath);
      if (path_opt.has_value()) {
        this->state_->accept_subscription(
          path_opt.value().get(), instance_data.instance_id);
      } else {
        break;
      }
    }
  }

  /** @brief removes references instance from all
   *
   */
  void RenderTree::delete_instance(widget_instance_id_t instance_id) {
    this->widget_instances_to_update_.erase(instance_id);
    auto it = this->render_tree_.find(instance_id);
    if (it != this->render_tree_.end()) {

      this->remove_subscriptions_from_state(it->second);
      for (auto [key, child] : it->second.children) {
        (void)key;
        this->deletion_set_.insert(child);
      }

      auto nerased = this->render_tree_.erase(instance_id);
      assert(nerased == 1);
    }
    this->deletion_set_.erase(instance_id);
  }

  WidgetInstanceData *RenderTree::insert_instance(
    widget_instance_id_t instance_id, WidgetInstanceData&& data) {
    // Todo dispose of the old widgetInstance
    this->delete_instance(instance_id);

    auto [inserter, ok] =
      this->render_tree_.insert({instance_id, std::move(data)});
    assert(ok && "insertion of new widget failed");
    this->widget_instances_to_update_.erase(instance_id);
    for (std::size_t ipath = 0;; ipath++) {
      std::optional<std::reference_wrapper<const std::string>> path_opt =
        inserter->second.widget->get_nth_selector_address(ipath);
      if (path_opt.has_value()) {
        this->state_->accept_subscription(path_opt.value().get(), instance_id);
      } else {
        break;
      }
    }
    this->empty_deletion_stack();
    return &inserter->second;
  }

  void RenderTree::set_render_node(
    widget_instance_id_t id, WidgetInstanceData&& new_node) {
    this->render_tree_[id] = std::move(new_node);
  }

  bool RenderTree::has_to_be_updated(widget_instance_id_t instance_id) const {
    auto it = this->widget_instances_to_update_.find(instance_id);
    return it != this->widget_instances_to_update_.end();
  }

} // namespace furender
