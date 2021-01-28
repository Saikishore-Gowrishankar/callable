
//Callable.h
//----------
//Custom callable type for compatibility with templates such as std::packaged_task
//
//std::function requires that the internal callable be CopyConstructible.
//In the case of std::packaged_task, which is a move_only type, this is not
//possible. This simple generic class solves the issue

/*Copyright (c) Saikishore Gowrishankar 2020. All rights reserved.
 *Licence: MIT*/

#pragma once

//Standard includes
#include <memory>       //Smart ptrs
#include <utility>      //std::move, std::forward
#include <type_traits>  //std::is_copy_constructible

//Dependent includes
#include "callable_traits.h"

template<typename Signature> class callable;
#define f_type(name) decltype(name)

template<typename R, typename... Args>
class callable<R(Args...)>
{
    struct callable_bridge_base
    {
        //Polymorphic clone, emulating covariance with name hiding private member
        [[nodiscard]]
        std::unique_ptr<callable_bridge_base> clone() const { return this->clone_impl(); }
        virtual R invoke(Args...) =0;                                   //Invoke callable
        virtual bool equals(std::unique_ptr<callable_bridge_base>) =0;  //Check equality of stored callable
        virtual ~callable_bridge_base() =default;                       //virtual dtor
    private:
        virtual callable_bridge_base* clone_impl() const =0;            //To implement covariance through smart pointers
    };

    template<typename F>
    struct callable_bridge_spec final : public callable_bridge_base
    {
        F f_; //Callable

        //Set callable
        callable_bridge_spec(auto&& func) : f_(std::forward<f_type(func)>(func)){}

        [[nodiscard]]
        std::unique_ptr<callable_bridge_spec> clone() const { return this->clone_impl(); }
        virtual R invoke(Args... args)  override {return f_(std::forward<Args>(args)...);}

        //A type without operator== will not cause a compilation failure until equals is actually called
        virtual bool equals(std::unique_ptr<callable_bridge_base> ptr) override
        {
            return equal_comp<F>::equals(f_,
                            static_cast<callable_bridge_spec<F> const *>(ptr.get())->f_);
        }
    private:
        virtual callable_bridge_spec* clone_impl() const override
        {
            if constexpr(std::is_copy_constructible_v<F>)
                return new callable_bridge_spec(f_);
            return nullptr;
        }
    };

    //Bridge to implementation
    std::unique_ptr<callable_bridge_base> bridge{nullptr};
public:
    //Exception safe swap
    friend void swap(callable& lhs, callable& rhs) noexcept
    {
        using std::swap;
        swap(lhs.bridge, rhs.bridge);
    }

    //Special members
    callable() = default;
    callable(callable const& rhs) { if(rhs.bridge) bridge = rhs.bridge->clone();}
    callable(callable&& rhs) : bridge{ std::move(rhs.bridge) } {}
    callable& operator=(callable rhs) noexcept { swap(*this, rhs); return *this;}

    //Sets callable object
    callable(auto&& f_)
        : bridge{std::make_unique<callable_bridge_spec<
                            f_type(f_)>>(std::forward<decltype(f_)>(f_) )}{}
    callable& operator=(auto&& f_) noexcept
    {
        swap(*this, callable(std::forward<f_type(f_)>(f_)));
        return *this;
    }

    //Invoke callable
    R operator() (Args... args) const {return bridge->invoke(std::forward<Args>(args)...);}

    //Boolean comparison of callable
    explicit operator bool(){ return bridge; }
    friend bool operator==(callable const& lhs, callable const& rhs)
    {
        bool test = !lhs || !rhs;
        return test?!lhs&&!rhs:lhs.bridge->equals(rhs.bridge);
    }
    friend bool operator!=(callable const& lhs, callable const& rhs){return !(lhs==rhs);}
};

//Factory
template<typename... Args>
auto callable_factory(auto&& f_)
{
    return callable<std::invoke_result_t<
                    decltype(f_), Args...>(Args...)>(
                        std::forward<f_type(f_)>(f_));
}
