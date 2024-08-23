#ifndef FLUXPP_WIDGET_HPP
#define FLUXPP_WIDGET_HPP

#include <tuple>
#include <memory>
#include <functional>
#include <type_traits>

#include  "selector.hpp"

namespace fluxpp {
  template <class ... arg_ts_>
  using  template_list = std::tuple <arg_ts_ ... >;  

  using widget_id_t = int64_t ;
  
  enum class WidgetType{
    Application,
    Client,
    Visual2D
  };

  template <WidgetType >
  class Context;

  template <WidgetType, class >
  class Widget ;
  
  namespace detail{
    template <WidgetType, class >
    class WidgetImplBase;

    template<WidgetType widget_type_, class return_t_, class... arg_ts_>
    class WidgetImplBase<widget_type_, return_t_(Context<widget_type_>& ,const  arg_ts_ &...)> {
    private:
      std::tuple<Selector<arg_ts_>...> selectors_ ;
    public:
      WidgetImplBase(std::tuple<Selector<arg_ts_>...> selectors): selectors_(std::move(selectors)){}
      
      const std::tuple<Selector<arg_ts_>...> & selectors(){
	return this->selectors_;
      }
      
      virtual return_t_ render(Context<widget_type_> & context, const arg_ts_ & ... args)=0;

      virtual ~WidgetImplBase() = default;
    };

    template <WidgetType, class callable_t_, class enable_t_, class fn_t_ >
    class WidgetImplDerived;


    template <WidgetType widget_type_,
	      class callable_t_,
	      class return_t_,
	      class ... arg_ts_>
    class WidgetImplDerived<
      widget_type_,
      callable_t_,
      typename std::enable_if<
	std::is_invocable_r<return_t_, callable_t_, Context<widget_type_>&, const arg_ts_ & ... >::value
	>::type,
      return_t_(Context<widget_type_> &,
		const arg_ts_ & ...)
      > final: public WidgetImplBase<widget_type_, return_t_(Context<widget_type_> &, const arg_ts_  & ...)>{
    private:
      callable_t_ callable_;
    public:
      WidgetImplDerived(callable_t_ callable,
			std::tuple<Selector<arg_ts_>...> selectors):
	WidgetImplBase<widget_type_,  return_t_(Context<widget_type_> &, const arg_ts_  & ...)>(std::move(selectors)),callable_(std::move(callable)) {};

      return_t_ render(Context<widget_type_>& context, const arg_ts_ & ... args) final override{
	return this->callable_(context, args...);
      }
    };
    
    widget_id_t create_widget_id();

    template <WidgetType widget_type_, class callable_t_, class return_t_, class ... arg_ts_>
    class WidgetBuilderS2{
    private:
      callable_t_ fn_;
      std::tuple<Selector<arg_ts_>...> selectors_;
    public:
      
      WidgetBuilderS2(callable_t_ fn, std::tuple<Selector<arg_ts_>...> selectors):
	fn_(std::move(fn)),selectors_(std::move(selectors)){}

      Widget<widget_type_, return_t_(Context<widget_type_>&, const arg_ts_ & ...) > build(){
	std::shared_ptr<WidgetImplBase<widget_type_, return_t_(Context<widget_type_>&, const arg_ts_ &...)>> ptr = std::make_shared<WidgetImplDerived<widget_type_,callable_t_, void, return_t_(Context<widget_type_>&, const arg_ts_ &...)>>(std::move(this->fn_), std::move(this->selectors_));
	
	return Widget<widget_type_, return_t_(Context<widget_type_>&, const arg_ts_& ...) >(std::move(ptr));
      }
    };
    
    template <WidgetType widget_type_, class ... arg_ts_>
    class WidgetBuilderS1{
    public:
      using arg_list_t = template_list <arg_ts_...>;
    private:
      std::tuple<Selector<arg_ts_>... > selectors;
    public:
      WidgetBuilderS1(Selector<arg_ts_>... args ):
	selectors(std::make_tuple( std::move(args)...)){ }

      template <class callable_t>
      WidgetBuilderS2<widget_type_,
		      callable_t,
		      typename std::invoke_result<callable_t,
					 Context<widget_type_>&,
					 const arg_ts_& ...>::type,
		      arg_ts_...>
      with_render_function(callable_t fn){
	using return_t  = typename std::invoke_result<callable_t,
						      Context<widget_type_>&,
						      const arg_ts_& ...>::type;
	return WidgetBuilderS2< widget_type_, callable_t, return_t, arg_ts_... >(std::move(fn), std::move(this->selectors));
      }
      
    };    
  }

  template <WidgetType widget_type_, class ... arg_ts_>
  detail::WidgetBuilderS1<widget_type_, arg_ts_ ...>
  create_widget_with_selectors(Selector<arg_ts_>... selectors){
    return detail::WidgetBuilderS1<widget_type_, arg_ts_...>(std::move(selectors)...);
  }

  

  /** @brief a widget is a (pureish) render function and selectors referencing state
   *
   *  Widget is a wrapper around a shared pointer to an (hidden) implementation
   */
  template<WidgetType widget_type_, class return_t_, class... arg_ts_>
  class Widget<widget_type_, return_t_(Context<widget_type_>&, const arg_ts_ &...)> {
  private:
    //    friend class detail::WidgetBuilderS2<widget_type_,return_t_,  arg_ts_...>;
  public:
    using return_t = return_t_;
    using arg_list_t = template_list <arg_ts_...>;
  private:
    std::shared_ptr<detail::WidgetImplBase<widget_type_, return_t_(Context<widget_type_>& ,const arg_ts_ &...)>> impl_;
  public:
    explicit Widget(std::shared_ptr<detail::WidgetImplBase<widget_type_, return_t_(Context<widget_type_>& ,const arg_ts_ &...)>> ptr):impl_(std::move(ptr)){};
    
  };

  
}

#endif // FLUXPP_WIDGET_HPP
