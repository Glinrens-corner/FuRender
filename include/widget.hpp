#ifndef FURENDER_WIDGET_HPP
#define FURENDER_WIDGET_HPP

#include <cstddef>
#include <string_view>
#include <tuple>
#include <memory>
#include <functional>
#include <type_traits>
#include <optional>
#include <utility>

#include "render_visitor.hpp"
#include "selector.hpp"
#include "basic.hpp"
#include "context.hpp"
#include "id_types.hpp"

namespace furender {

  /** @brief virtual base class for Widgets
   *
   */
  class BaseWidget {
    private:
    widget_id_t widget_id_ ;
  public:
    BaseWidget(widget_id_t widget_id):widget_id_(widget_id){}
    virtual  void accept(RenderVisitor &  visitor)=0;

    virtual ~BaseWidget()=default;

    virtual std::optional<std::reference_wrapper<const std::string>> get_nth_selector_address(std::size_t i)=0;

    widget_id_t get_widget_id()const{
      return this->widget_id_;
    }
  };



  /** @brief virtual base class for Widgets that knows its WidgetType
   *
   */
  template <WidgetType widget_type_>
  class DeferredWidget:public BaseWidget{
  public:
    DeferredWidget(widget_id_t widget_id):BaseWidget(widget_id){};
  };



  /** @brief WidgetClass, that knows its Widgettype and the return type of the render function
   *
   */
  template <WidgetType widget_type_, class return_t_  >
  class Widget : public DeferredWidget<widget_type_>{
  public:
    Widget(widget_id_t widget_id):DeferredWidget<widget_type_>(widget_id){}



  };

  template <WidgetType widget_type_, class render_fn_t_, class render_sig_t_>
  class WidgetImpl;



  /** @brief final Widget class adheres to a concept
   *
   * the final Widget class needs to override accept() such that is calls visitor.render(*this);
   *  * also needs to override  get_nth_selector_address method as well as
   *  * implement return_t as the return type of the render method
   *  * implement args_tuple_t as a std::tuple<arg_ts...> of the Selector arguments
   *  * implement return_t render(Context<widget_type>& , const arg_ts_&...)
   */
  template <WidgetType widget_type_, class render_fn_t_, class return_t_, class ...arg_ts_>
  class WidgetImpl<widget_type_, render_fn_t_ , return_t_(Context<widget_type_>&, const arg_ts_ &...)> final:
    public Widget<widget_type_, return_t_>{
  public:
    using return_t = return_t_;
    using args_tuple_t = std::tuple<  arg_ts_ ...>;
  private:
    render_fn_t_ render_fn_;
    std::tuple<Selector<arg_ts_>...> selectors_ ;



  public:
    WidgetImpl(render_fn_t_ render_fn,std::tuple<Selector<arg_ts_>...> selectors, widget_id_t widget_id)
      :Widget<widget_type_, return_t_>(widget_id),
       render_fn_(std::move(render_fn)),
       selectors_(std::move(selectors))
    {}



    constexpr static WidgetType get_widget_type(){
      return widget_type_;
    }



    void accept(RenderVisitor & visitor) final override{
      visitor.render(*this);
    }



    std::optional<std::reference_wrapper<const std::string>> get_nth_selector_address(std::size_t i) final override{
      return get_nth_selector_address_i<0>( i);
    }



    const std::tuple<Selector<arg_ts_>...> & selectors(){
      return this->selectors_;
    }



    return_t_ render(Context<widget_type_>& context, const arg_ts_& ... args){
      return this->render_fn_(context, args...);
    }



  private:
    template <std::size_t I>
    std::optional<std::reference_wrapper<const std::string>> get_nth_selector_address_i(std::size_t i){
      if constexpr( I < std::tuple_size_v<std::tuple<Selector<arg_ts_>...>>){
	if(I==i){
	  return std::get<I>(this->selectors_).path();
	} else {
	  return get_nth_selector_address_i<I+1>(i);
	}
      }else {
	return {};
      }
    }



  private:
    static_assert(std::is_same_v<
		  std::invoke_result_t<render_fn_t_, Context<widget_type_>&, const arg_ts_ &...>
		  , return_t_>, "the render_function has to comfort to the render_function_signature");
  };

  namespace detail{

    widget_id_t create_widget_id();

    template <WidgetType widget_type_, class render_fn_t_, class return_t_, class ... arg_ts_>
    class WidgetBuilderStage2{
    private:
      render_fn_t_ fn_;
      std::tuple<Selector<arg_ts_>...> selectors_;
    public:

      WidgetBuilderStage2(render_fn_t_ fn, std::tuple<Selector<arg_ts_>...> selectors):
	fn_(std::move(fn)),selectors_(std::move(selectors)){}

      std::shared_ptr<
	WidgetImpl<
	widget_type_,
	render_fn_t_,
	return_t_(Context<widget_type_>&, const arg_ts_ & ...) >>
      make_shared(){
	return
	  std::make_shared<
	    WidgetImpl<widget_type_,
		       render_fn_t_,
		       return_t_(Context<widget_type_>&, const arg_ts_ &...)>
	  >(std::move(this->fn_), std::move(this->selectors_), create_widget_id());
      }
    };

    template <WidgetType widget_type_, class ... arg_ts_>
    class WidgetBuilderStage1{
    public:
      using arg_list_t = template_list <arg_ts_...>;
    private:
      std::tuple<Selector<arg_ts_>... > selectors_;
    public:
      WidgetBuilderStage1(Selector<arg_ts_>... args ):
	selectors_(std::make_tuple( std::move(args)...)){ }

      template <class render_fn_t>
      WidgetBuilderStage2<widget_type_,
			  render_fn_t,
			  typename std::invoke_result<render_fn_t,
					 Context<widget_type_>&,
					 const arg_ts_& ...>::type,
			  arg_ts_...>
      with_render_function(render_fn_t fn){
	using return_t  = typename std::invoke_result<render_fn_t,
						      Context<widget_type_>&,
						      const arg_ts_& ...>::type;
	return WidgetBuilderStage2< widget_type_, render_fn_t, return_t, arg_ts_... >(std::move(fn), std::move(this->selectors_));
      }

    };




  }

  /** @brief creates a WidgetBuilder that builds widget implementation.
   *
   */
  template <WidgetType widget_type_, class ... arg_ts_>
  detail::WidgetBuilderStage1<widget_type_, arg_ts_ ...> create_widget_with_selectors(Selector<arg_ts_>... selectors){
    return detail::WidgetBuilderStage1<widget_type_, arg_ts_...>(std::move(selectors)...);
  }

}

#endif // FURENDER_WIDGET_HPP
