#ifndef FLUXPP_WIDGET_HPP
#define FLUXPP_WIDGET_HPP

#include <cstddef>
#include <string_view>
#include <tuple>
#include <memory>
#include <functional>
#include <type_traits>
#include <optional>
#include <utility>

#include "selector.hpp"
#include "basic.hpp"
#include "context.hpp"

namespace fluxpp {

  using widget_id_t = uint64_t ;

  template<WidgetType widget_type_>
  class RenderVisitor{
  public:
    template< class T>
    void render(T*){};
    
  };

  template<WidgetType widget_type_, class return_t_>
  class TypedRenderVisitor{
  public:
    template< class T>
    return_t_ render(T*){
      if constexpr (std::is_same_v<return_t_,void>){
	return;
      } else {
	return {};
      }
    };
    
  };
  

  template <WidgetType widget_type_>
  class ErasedWidget{
  private:
    widget_id_t widget_id_ ;
  public:
    ErasedWidget(widget_id_t widget_id):widget_id_(widget_id){}
    virtual  void accept(RenderVisitor<widget_type_> &  visitor)=0;
    virtual ~ErasedWidget()=default;
  };

    
  template <WidgetType widget_type_, class return_t_  >
  class Widget : public ErasedWidget<widget_type_>{
  public:
    Widget(widget_id_t widget_id):ErasedWidget<widget_type_>(widget_id){}
    
    
    virtual  return_t_ typed_accept(TypedRenderVisitor<widget_type_, return_t_ > &  visitor)=0;
   
  };

  template <WidgetType widget_type_, class render_fn_t_, class render_sig_t_>
  class WidgetImpl;


  
  template <WidgetType widget_type_, class render_fn_t_, class return_t_, class ...arg_ts_>
  class WidgetImpl<widget_type_, render_fn_t_ , return_t_(Context<widget_type_>&, const arg_ts_ &...)> final:
    public Widget<widget_type_, return_t_>{
  public:
    
  private:
    render_fn_t_ render_fn_;
    std::tuple<Selector<arg_ts_>...> selectors_ ;


    
  public:
    WidgetImpl(render_fn_t_ render_fn,std::tuple<Selector<arg_ts_>...> selectors, widget_id_t widget_id)
      :Widget<widget_type_, return_t_>(widget_id),
       render_fn_(std::move(render_fn)),
       selectors_(std::move(selectors))
    {}


      
    void accept(RenderVisitor<widget_type_> & visitor) final override{
      visitor.render(this);
    }


    
    return_t_ typed_accept(TypedRenderVisitor<widget_type_, return_t_ > & visitor) final override{
      return visitor.render(this);  
    }

    
    
    std::optional<std::string_view> get_nth_selector_address(std::size_t i){
      return get_nth_selector_address_i( i);
    }


    
    const std::tuple<Selector<arg_ts_>...> & selectors(){
      return this->selectors_;
    }



    return_t_ render(Context<widget_type_>& context, const arg_ts_& ... args){
      return this->render_fn_(context, args...);
    }


    
  private:
    template <std::size_t I>
    std::optional<std::string_view>    get_nth_selector_address_i(std::size_t i){
      if constexpr( I < std::tuple_size_v<std::tuple<Selector<arg_ts_>...>>){
	if(I==i){
	  return std::string_view(std::get<I>(this->selectors_).path());
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

    uint64_t create_widget_id();
    
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

  template <WidgetType widget_type_, class ... arg_ts_>
  detail::WidgetBuilderStage1<widget_type_, arg_ts_ ...>
  create_widget_with_selectors(Selector<arg_ts_>... selectors){
    return detail::WidgetBuilderStage1<widget_type_, arg_ts_...>(std::move(selectors)...);
  }
  
}  

#endif // FLUXPP_WIDGET_HPP
