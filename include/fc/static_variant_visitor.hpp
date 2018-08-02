#pragma once

#include <fc/functor_traits.hpp>
#include <fc/optional.hpp>

namespace fc {
namespace impl {
/**
 * 'strict' visitor requires all variants provided in functor(s) when you are calling 'variant.visit([functor(s)/lambdas])'
 *
 * 'T' and 'Ts' here are functor(s) or lambdas (lambda is just a struct with operator() i.e. also functor).
 *
 * In C++17 this code can be dramatically simplified cuz there we have pack-expansion in using-declarations,
 * i.e. using Ts::operator()...;
 */
template <typename T, typename... Ts> struct strict_visitor : T, strict_visitor<Ts...>
{
    using T::operator();
    using strict_visitor<Ts...>::operator();

    template <typename U, typename... Us>
    strict_visitor(U&& f, Us&&... fs)
        : T(std::forward<U>(f))
        , strict_visitor<Ts...>(std::forward<Us>(fs)...) { }
};

template <typename T> struct strict_visitor<T> : T
{
    using T::operator();

    template <typename U>
    strict_visitor(U&& f)
        : T(std::forward<U>(f)) { }
};

template <typename T, typename = void>
struct weak_visitor_base
{
    using result_type = void;
    template <typename U> void operator()(const U&) const { }
};

template <typename T>
struct weak_visitor_base<T, typename std::enable_if<!std::is_void<typename functor_traits<T>::return_type>::value>::type>
{
    using result_type = fc::optional<typename functor_traits<T>::return_type>;
    template <typename U> result_type operator()(const U&) const { return {}; }
};
/**
 * 'weak' visitor DO NOT require all variants provided in functor when you are calling 'variant.visit([functor/lambdas])'
 *
 * 'T' and 'Ts' here are functor(s) or lambdas (lambda is just a struct with operator() i.e. also functor).
 *
 * In C++17 this code can be dramatically simplified cuz there we have pack-expansion in using-declarations,
 * i.e. using Ts::operator()...;
 */
template <typename T, typename... Ts> struct weak_visitor : T, weak_visitor<Ts...>
{
    using T::operator();
    using weak_visitor<Ts...>::operator();

    template <typename U, typename... Us>
    weak_visitor(U&& f, Us&&... fs)
        : T(std::forward<U>(f))
        , weak_visitor<Ts...>(std::forward<Us>(fs)...) { }
};

template <typename T> struct weak_visitor<T> : T, weak_visitor_base<T>
{
    using T::operator();
    /**
     * This is the difference between 'strict_visitor' and 'weak_visitor'.
     * 'weak_visitor' will call this operator if it obtains variant which doesn't match any of the provided functors.
     */
    using weak_visitor_base<T>::operator();

    template <typename U>
    weak_visitor(U&& f)
        : T(std::forward<U>(f)) { }
};
}

template <typename T, typename... Ts>
auto make_strict_visitor(T&& f, Ts&&... fs)
{
    return impl::strict_visitor<typename std::decay<T>::type, typename std::decay<Ts>::type...>(std::forward<T>(f), std::forward<Ts>(fs)...);
}

template <typename T, typename... Ts>
auto make_weak_visitor(T&& f, Ts&&... fs)
{
    return impl::weak_visitor<typename std::decay<T>::type, typename std::decay<Ts>::type...>(std::forward<T>(f), std::forward<Ts>(fs)...);
}
}