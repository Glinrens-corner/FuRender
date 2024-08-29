#ifndef FLUXPP_STATE_HPP
#define FLUXPP_STATE_HPP

#include <tuple>
#include <type_traits>
//#include <typeinfo>
#include <memory>
#include <functional>
#include <unordered_map>

#include "basic.hpp"
#include "event.hpp"
#include "addressor.hpp"

namespace fluxpp{
  enum class UpdateType{
    ChangingOnEvent,
    //    MemComparable,
    EqualityComparable
  };


  
  template<class event_t_, class invocation_t_>
  struct is_data_event_handling: std::false_type{
  };
  
  template<class data_t_>
  struct is_data_event_handling<DataEvent<data_t_>, data_t_>
    :std::true_type{
  };
  
  template<class event_t_, class invocation_t_>
  struct is_signal_event_handling: std::false_type{
  };
  
  template<>
  struct is_signal_event_handling<SignalEvent, void>{
    static constexpr  bool value() {return true;}
  };
  
  template <bool , bool >
  struct and_type : std::false_type{};

  template < >
  struct and_type<true,true> : std::true_type{};

    
  template<class event_t_, class invocation_t_>
  struct is_common_event_handling:
    and_type <!is_data_event_handling< event_t_,  invocation_t_>::value,
	      !is_signal_event_handling< event_t_,  invocation_t_>::value>{
  };

  template <class state_t>
  class TypedStateSlice;
    
  template< class state_t_ >
  class StateContext{
  private:
    UpdateType update_type_ = UpdateType::ChangingOnEvent;
    TypedStateSlice<state_t_>* slice_;;


  public:
    StateContext(TypedStateSlice<state_t_>* slice, UpdateType update_type):update_type_(update_type), slice_(slice){};

    void update_state(state_t_ new_state);
  };

  
  class StateDispatchVisitor{
  private:
    EventEnvelope event_envelope_;
  public:
    StateDispatchVisitor(EventEnvelope event_envelope):event_envelope_(std::move(event_envelope)){};
    
    template <class state_slice_impl_t>
    void visit(state_slice_impl_t* state_slice_impl){
      using state_t = typename state_slice_impl_t::state_t;
      UpdateType update_type = state_slice_impl->update_type();
      StateContext<state_t> context(state_slice_impl,  update_type);
      
      const EventEnvelope & const_event_envelope = this->event_envelope_;
      state_slice_impl->handle_event(context, const_event_envelope);
    } 
  };
  

  
  class StateSlice{
  public:
    virtual void accept_dispatch_visitor(StateDispatchVisitor& ) = 0;
    virtual ~StateSlice(){};
  };




  
  template< class state_t_>
  class TypedStateSlice: public StateSlice{
  public:
    using state_t = state_t_;
  protected:
    state_t state_{};
  public:
    explicit TypedStateSlice(state_t state):StateSlice(),state_t_(std::move(state)){  
    }



    TypedStateSlice(){  
    }



    virtual state_t* get_state_ptr(){
      return &this->state_;
    }


    
    virtual void set_state(state_t_ state){
      this->state_ = state;
    }


    

  private:
    static_assert(std::is_default_constructible_v<state_t_>,"state must be default constructible" );
    
  };


  

  
  template<class ,class   ,class,  UpdateType=UpdateType::ChangingOnEvent>
  class ReducingStateSlice;



  template<class state_t_, class initializer_t_, class...invocation_ts_, class ... event_ts_,class... callable_ts_, UpdateType update_type_ >
  class ReducingStateSlice<
    state_t_,
    initializer_t_,
    template_list<
      template_list<event_ts_ ...>,
      template_list<invocation_ts_...>,
      template_list<callable_ts_...>> ,
      update_type_>:  public TypedStateSlice<state_t_>{
  private:
    initializer_t_ initializer_;
    std::tuple<callable_ts_...> reducers_;
    
  public:
    ReducingStateSlice(
		       initializer_t_&& initializer,
		       std::tuple<callable_ts_...> reducers):
      TypedStateSlice<state_t_>(),
      initializer_(std::move(initializer)),
      reducers_(std::move(reducers)){};
    
    void initialize(){
      this->state_ =  this->initializer_();
    }
    
    void accept_dispatch_visitor(StateDispatchVisitor& visitor) override final{
      visitor.visit(this);
    }

    void handle_event(StateContext< state_t_ > & context, const EventEnvelope& envelope ){
      try_next_handler<0>(context, envelope);
    }



    static constexpr UpdateType update_type(){
      return update_type_;
    }


    
  private:
    template <std::size_t i>
    typename std::enable_if<i < sizeof...(callable_ts_), void>::type
    try_next_handler(StateContext<state_t_> & context, const EventEnvelope& envelope){
      if (try_nth_handler<i>(context,envelope)){
	return ;
      } else {
	try_next_handler<i+1>( context, envelope);
      };
    };


    
    template <std::size_t i>
    typename std::enable_if<i == sizeof...(callable_ts_), void>::type try_next_handler(StateContext<state_t_> & , const EventEnvelope& ){
    };
    

    
    template<std::size_t i>
    bool try_nth_handler(StateContext< state_t_> & context, const EventEnvelope & envelope ){
      using event_t = typename template_list_element< i, template_list<event_ts_...> >::type;
      using invocation_t = typename template_list_element< i, template_list<invocation_ts_...> >::type;
      event_t* event_ptr = dynamic_cast<event_t*>(envelope.event );
      //      error<invocation_t>::value;
      if (event_ptr != nullptr){
	if(this->handle_data_event<i, event_t, invocation_t>(context, *event_ptr))return true;
	if(handle_signal_event<i, event_t, invocation_t>(context, *event_ptr))return true;
	if(handle_common_event<i, event_t, invocation_t>(context, *event_ptr))return true;
      }
      return false;
    };


    
    template <std::size_t i, class event_t, class invocation_t >
     bool handle_data_event(StateContext< state_t_> & context, const event_t & event ){
      if constexpr (is_data_event_handling<event_t, invocation_t>::value){
	std::get<i>(this->reducers_ )(context, event.data);
	return true;
      } else {
	return false;
      }
    }



    template <std::size_t i,
	      class event_t,
	      class invocation_t
	      >
    bool handle_signal_event(StateContext< state_t_> & context, const event_t & event ){
      if constexpr (is_signal_event_handling<event_t, invocation_t>::value ){
      std::get<i>(this->reducers_ )(context);
      return true;
      } else {
	return false;
      }
    }


      
    
      

    template <std::size_t i=0, class event_t = initializer_t_, class invocation_t  = initializer_t_>
    bool handle_common_event(StateContext< state_t_> & context, const event_t & event ){
      if constexpr (is_common_event_handling<event_t, invocation_t>::value ){ 
	std::get<i>(this->reducers_ )(context, event);
	return true;
      } else {

	return false;
      }
    }
    
  };

  namespace detail{
    template<class, class, class, UpdateType >
    class ReducingStateSliceBuilder;

    template <
      class state_t_,
      class initializer_t_,
      class... event_ts_,
      class ... invocation_ts_,
      class ... reducer_ts_,
      UpdateType update_type_>
    class ReducingStateSliceBuilder<
      state_t_ ,
      initializer_t_,
      template_list<
	template_list<event_ts_...>,
	template_list<invocation_ts_...>,
	template_list<reducer_ts_...>
	>,
      update_type_>{
    private:
      initializer_t_ initializer_;
      std::tuple<reducer_ts_...> reducers_;
    public:
      ReducingStateSliceBuilder(initializer_t_ &&initializer, std::tuple<reducer_ts_...> && reducers ):
	initializer_(std::move(initializer)),
	reducers_ (std::move(reducers)){};

      template<class event_t,class reducer_t>
      auto with_event_reducer(reducer_t reducer ){
	
	using namespace std;
	using new_builder_t = ReducingStateSliceBuilder<
	  state_t_,
	  initializer_t_,
	  template_list<
	    template_list<event_ts_..., event_t>,
	  template_list<invocation_ts_..., event_t>,
	  template_list<reducer_ts_..., reducer_t>
	  >,
	  update_type_>;
	
	return new_builder_t(
			     move(this->initializer_),
			     tuple_cat(
				       move(this->reducers_),
				       std::make_tuple(move(reducer))) );
      }



      template<class data_t,class reducer_t>
      auto with_data_reducer(reducer_t reducer ){
	using namespace std;
	using new_builder_t = ReducingStateSliceBuilder<
	  state_t_,
	  initializer_t_,
	  template_list<
	    template_list<event_ts_..., DataEvent<data_t>>,
	    template_list<invocation_ts_..., data_t>,
	    template_list<reducer_ts_..., reducer_t>
	  >,
	  update_type_>;
	
	return new_builder_t(
			     std::move(this->initializer_),
			     tuple_cat(
				       std::move(this->reducers_),
				       make_tuple(std::move(reducer))) );
      }


      
      template<class event_t,class reducer_t>
      auto with_signal_reducer(reducer_t reducer ){
	using namespace std;
	using new_builder_t = ReducingStateSliceBuilder<
	  state_t_,
	  initializer_t_,
	  template_list<
	    template_list<event_ts_..., SignalEvent>,
	  template_list<invocation_ts_..., void>,
	  template_list<reducer_ts_..., reducer_t>
	  >,
	  update_type_>;
	
	return new_builder_t(
			     move(this->initializer_),
			     tuple_cat(
				       move(this->reducers_),
				       make_tuple(move(reducer))) );
      }



      auto make_unique_ptr(){
	
	auto u_ptr= std::unique_ptr<
	  ReducingStateSlice<
	    state_t_,
	    initializer_t_,
	    template_list<
	      template_list<event_ts_...>,
	      template_list<invocation_ts_...>,
	      template_list<reducer_ts_...>
	      > >
	  >(new ReducingStateSlice<
	    state_t_,
	    initializer_t_,
	    template_list<
	      template_list<event_ts_...>,
	      template_list<invocation_ts_...>,
	      template_list<reducer_ts_...>
	    > >(std::move(this->initializer_), std::move(this->reducers_)));
	u_ptr-> initialize();
	return std::move(u_ptr);
      };
    };
  }

  template<class initializer_t>
  auto create_state_slice(initializer_t initializer){
    using state_t = std::invoke_result_t<initializer_t>;
    static_assert(! std::is_void_v<state_t> );
    return detail::ReducingStateSliceBuilder<
      state_t,
      initializer_t,
      template_list<
      template_list<>,
      template_list<>,
      template_list<>
	>
      , UpdateType::ChangingOnEvent>(std::move(initializer),std::tuple());
  }



  class State{
  private:
    std::unordered_map<std::string,std::unique_ptr<StateSlice>> slices_;
  public:
    template<class selector_t>
    const typename selector_t::selected_t* get_state_ptr(const selector_t & selector){
      using state_t = typename selector_t::selected_t;
      StateSlice* slice = this->get_state_slice(selector.path());
      TypedStateSlice<state_t>* slice_typed = dynamic_cast<TypedStateSlice<state_t>*>( slice);
      if(slice_typed){
	return slice_typed->get_state_ptr();
      }else{
	//Error incorrect type requested
	return nullptr;
      }
    }
    


    template<class state_t, class ... event_ts>
    void set_state_slice(Addressor<state_t, event_ts...>& addressor,
			 std::unique_ptr<StateSlice> slice ){
      if (!this->slices_.insert({addressor.path(), std::move(slice)}).second){
	throw "Error problem accepting a new state slice";
      };
    }


    
    void set_state_slice(std::string path, std::unique_ptr<StateSlice> slice){
      if (!this->slices_.insert({std::move(path), std::move(slice)}).second){
	throw "Error problem accepting a new state slice";
      };
      
    }

    
    
    StateSlice*  get_state_slice(const std::string & path) ;
    
  };
  
  template<class state_t_>
  void StateContext<state_t_>::update_state(state_t_ new_state) {
    this->slice_->set_state(new_state);
  }
}

    


#endif //FLUXPP_STATE_HPP
