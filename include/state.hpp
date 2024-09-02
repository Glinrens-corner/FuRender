#ifndef FLUXPP_STATE_HPP
#define FLUXPP_STATE_HPP

#include <cassert>
#include <tuple>
#include <type_traits>
//#include <typeinfo>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

#include "basic.hpp"
#include "event.hpp"
#include "addressor.hpp"
#include "id_types.hpp"

namespace fluxpp{
  class RenderTree;

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
    TypedStateSlice<state_t_>* slice_;;
    bool is_updated_;

  public:
    StateContext(TypedStateSlice<state_t_>* slice):
      slice_(slice){
      this->is_updated_ = this->get_initial_is_updated_value();
    };



    bool is_updated(){
      return this->is_updated_;
    }
  public:
    void update_state(state_t_ new_state);

  private:
    bool get_initial_is_updated_value()const;
  };




  
  class StateDispatchVisitor{
  private:
    EventEnvelope event_envelope_;
  public:
    StateDispatchVisitor(EventEnvelope event_envelope):event_envelope_(std::move(event_envelope)){};
    
    template <class state_slice_impl_t>
    bool visit(state_slice_impl_t* state_slice_impl){
      const BaseEvent&  event= *this->event_envelope_.event;
      return state_slice_impl->handle_event(event);
    } 
  };
  

  
  class StateSlice{
  public:
    virtual bool accept_dispatch_visitor(StateDispatchVisitor& ) = 0;
    virtual ~StateSlice(){};
    virtual UpdateType update_type()=0;
  };




  
  template< class state_t_>
  class TypedStateSlice: public StateSlice{
  public:
    using state_t = state_t_;
  protected:
    state_t state_{};
  public:
    explicit TypedStateSlice(state_t state):StateSlice(),state_(std::move(state)){  
    }



    TypedStateSlice(){  
    }



    virtual state_t* get_state_ptr(){
      return &this->state_;
    }


    
    virtual bool set_state(state_t_ state){
      this->state_ = state;
      return false;
    }


    
    const state_t_ & state()const{
      return this->state_;
    }
  private:
    static_assert(std::is_default_constructible_v<state_t_>,"state must be default constructible" );
    
  };


  

  
  template<class, class,  UpdateType=UpdateType::ChangingOnEvent>
  class ReducingStateSlice;



  template<class state_t_, class...invocation_ts_, class ... event_ts_,class... callable_ts_, UpdateType update_type_ >
  class ReducingStateSlice<
    state_t_,
    template_list<
      template_list<event_ts_ ...>,
      template_list<invocation_ts_...>,
      template_list<callable_ts_...>> ,
      update_type_>:  public TypedStateSlice<state_t_>{
  public:
      using addressor_t = Addressor<state_t_, event_ts_...>; 
  private:
    std::tuple<callable_ts_...> reducers_;
    
  public:
    ReducingStateSlice(state_t_ initial_value,
		       std::tuple<callable_ts_...> reducers):
      TypedStateSlice<state_t_>(std::move(initial_value)),
      reducers_(std::move(reducers)){};
    
    bool accept_dispatch_visitor(StateDispatchVisitor& visitor) override final{
      return visitor.visit(this);
    }


    
    bool handle_event(const BaseEvent& event ){
      StateContext<state_t_>context{this};
      try_next_handler<0>(context, event);
      return context.is_updated();
    }


    
    UpdateType update_type()override final{
      return update_type_;
    }



    static addressor_t create_addressor(std::string path){
      return {std::move(path)};
    }



    bool set_state(state_t_ state)override final{
      bool updated = false;
      if constexpr(update_type_==UpdateType::ChangingOnEvent){
	updated = true;
      }else if constexpr(update_type_== UpdateType::EqualityComparable){
	updated = this->state()==state;
      };
      TypedStateSlice<state_t_>::set_state(state);
      return updated;
    }
  private:
    template <std::size_t i>
    typename std::enable_if<i < sizeof...(callable_ts_), void>::type
    try_next_handler( StateContext< state_t_>& context, const BaseEvent& event){

      if (try_nth_handler<i>(context, event)){
	return ;
      } else {
	try_next_handler<i+1>(context, event);
      };
    };


    
    template <std::size_t i>
    typename std::enable_if<i == sizeof...(callable_ts_), void>::type try_next_handler(StateContext< state_t_>& , const BaseEvent& ){
    };
    

    
    template<std::size_t i>
    bool try_nth_handler(StateContext< state_t_>&, const BaseEvent & event ){
      using event_t = typename template_list_element< i, template_list<event_ts_...> >::type;
      using invocation_t = typename template_list_element< i, template_list<invocation_ts_...> >::type;
      const event_t* event_ptr = dynamic_cast<const event_t*>(&event );
      if (event_ptr != nullptr){
	if(this->handle_data_event<i, event_t, invocation_t>(*event_ptr))return true;
	if(handle_signal_event<i, event_t, invocation_t>(*event_ptr))return true;
	if(handle_common_event<i, event_t, invocation_t>( *event_ptr))return true;
      }
      return false;
    };


    
    template <std::size_t i, class event_t, class invocation_t >
     bool handle_data_event( const event_t & event ){
      if constexpr (is_data_event_handling<event_t, invocation_t>::value){
	StateContext<state_t_>context{this};

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
    bool handle_signal_event(const event_t & event ){
      if constexpr (is_signal_event_handling<event_t, invocation_t>::value ){
      StateContext<state_t_>context{this};
      std::get<i>(this->reducers_ )(context);
      return true;
      } else {
	return false;
      }
    }



    template <std::size_t i, class event_t, class invocation_t  >
    bool handle_common_event( const event_t & event ){
      if constexpr (is_common_event_handling<event_t, invocation_t>::value ){ 
	StateContext<state_t_>context{this};
	std::get<i>(this->reducers_ )(context, event);
	return true;
      } else {

	return false;
      }
    }
    
  };

  namespace detail{
    template<class, class, UpdateType >
    class ReducingStateSliceBuilder;

    template <
      class state_t_,
      class... event_ts_,
      class ... invocation_ts_,
      class ... reducer_ts_,
      UpdateType update_type_>
    class ReducingStateSliceBuilder<
      state_t_ ,
      template_list<
	template_list<event_ts_...>,
	template_list<invocation_ts_...>,
	template_list<reducer_ts_...>
	>,
      update_type_>{
    private:
      state_t_ initial_value_;
      std::tuple<reducer_ts_...> reducers_;
    public:
      ReducingStateSliceBuilder(state_t_&& initial_value, std::tuple<reducer_ts_...> && reducers ):initial_value_(std::move(initial_value)),
	reducers_ (std::move(reducers)){};
      


      template<class event_t,class reducer_t>
      auto with_event_reducer(reducer_t reducer ){
	
	using namespace std;
	using new_builder_t = ReducingStateSliceBuilder<
	  state_t_,
	  template_list<
	    template_list<event_ts_..., event_t>,
	  template_list<invocation_ts_..., event_t>,
	  template_list<reducer_ts_..., reducer_t>
	  >,
	  update_type_>;
	
	return new_builder_t(std::move(this->initial_value_),
			     tuple_cat(
				       std::move(this->reducers_),
				       std::make_tuple(std::move(reducer))) );
      }



      template<class data_t,class reducer_t>
      auto with_data_reducer(reducer_t reducer ){
	using namespace std;
	using new_builder_t = ReducingStateSliceBuilder<
	  state_t_,
	  template_list<
	    template_list<event_ts_..., DataEvent<data_t>>,
	    template_list<invocation_ts_..., data_t>,
	    template_list<reducer_ts_..., reducer_t>
	  >,
	  update_type_>;
	
	return new_builder_t(std::move(this->initial_value_),
			     tuple_cat(
				       std::move(this->reducers_),
				       make_tuple(std::move(reducer))) );
      }


      
      template<class event_t,class reducer_t>
      auto with_signal_reducer(reducer_t reducer ){
	using namespace std;
	using new_builder_t = ReducingStateSliceBuilder<
	  state_t_,
	  template_list<
	    template_list<event_ts_..., SignalEvent>,
	  template_list<invocation_ts_..., void>,
	  template_list<reducer_ts_..., reducer_t>
	  >,
	  update_type_>;
	
	return new_builder_t(std::move(this->initial_value_),
			     tuple_cat(
				       std::move(this->reducers_),
				       make_tuple(std::move(reducer))) );
      }



      auto make_unique_ptr(){
	auto u_ptr= std::unique_ptr<
	  ReducingStateSlice<
	    state_t_,
	    template_list<
	      template_list<event_ts_...>,
	      template_list<invocation_ts_...>,
	      template_list<reducer_ts_...>
	      > >
	  >(new ReducingStateSlice<
	    state_t_,
	    template_list<
	      template_list<event_ts_...>,
	      template_list<invocation_ts_...>,
	      template_list<reducer_ts_...>
	    > >(std::move(this->initial_value_), std::move(this->reducers_)));
	return std::move(u_ptr);
      };
    };
  }



  template<class state_t>
  auto create_state_slice(state_t initial_value){
    static_assert(! std::is_void_v<state_t> );
    return detail::ReducingStateSliceBuilder<
      state_t,
      template_list<
      template_list<>,
      template_list<>,
      template_list<>
	>
      , UpdateType::ChangingOnEvent>(std::move(initial_value),std::tuple<>());
  }




  
  class State{
  
#ifdef NDEBUG
  private:
#else
  public:
#endif //NDEBUG
    struct SliceData{
      std::unique_ptr<StateSlice> slice;
      std::vector<widget_instance_id_t> subscriptions;
    };
#ifdef NDEBUG
  private:
#else
  public:
#endif //NDEBUG
    std::unordered_map<std::string,SliceData> slices_;
    RenderTree * render_tree_=nullptr;
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
      if (!this->slices_.insert({addressor.path(), SliceData{std::move(slice),{}}}).second){
	throw "Error problem accepting a new state slice";
      };
    }


    
    void set_state_slice(std::string path, std::unique_ptr<StateSlice> slice){
      if (!this->slices_.insert({std::move(path), SliceData{std::move(slice)}}).second){
	throw "Error problem accepting a new state slice";
      };
    }



    void accept_subscription(const std::string& path, widget_instance_id_t instance_id){
      auto it = this->slices_.find(path);
      if(it != this->slices_.end()){
	it->second.subscriptions.push_back(instance_id);
      }
    }


    
    void remove_subscription(const std::string& path, widget_instance_id_t instance_id){
      auto it = this->slices_.find(path);
      if(it != this->slices_.end()){
	std::vector<widget_instance_id_t>& subscriptions = it->second.subscriptions;
	//auto start_it = subscriptions.begin();
        auto current_it  =  std::find(subscriptions.begin(), subscriptions.end(), instance_id);
	if(current_it!= subscriptions.end()){
	  *current_it= subscriptions.back();
	  subscriptions.pop_back();
	}
      }
    }


    
    void set_render_tree(RenderTree* render_tree){
      this->render_tree_=render_tree;
    }
  public:
    StateSlice*  get_state_slice(const std::string & path) ;
    
    void dispatch_event(EventEnvelope& );
    
  };


  
  template<class state_t_>
  void StateContext<state_t_>::update_state(state_t_ new_state) {
    this->is_updated_ = this->slice_->set_state(new_state);
  }


  
  template<class state_t_>
  bool StateContext<state_t_>::get_initial_is_updated_value()const{
    switch(this->slice_->update_type()  ){
    case UpdateType::ChangingOnEvent:
      return true;
      break;;
    case UpdateType::EqualityComparable:
      return false;
      break;;
    }
    assert(false);
  }
}

    


#endif //FLUXPP_STATE_HPP
