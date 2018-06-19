#pragma once

#include <fc/functor_traits.hpp>

namespace fc {
namespace impl {
namespace cxx17 {
    template<class...>
    using void_t = void;
}

template<typename TF, typename = impl::cxx17::void_t<>>
struct has_result_type : std::false_type
{ };

template<typename TF>
struct has_result_type<TF, impl::cxx17::void_t<typename std::decay<TF>::type::result_type>> : std::true_type
{ };

/**
 * If 'T' functor doesn't have result_type which is required by fc::static_variant visitors (it is a lambda for example)
 * then declare it
 */
template<typename T, typename = cxx17::void_t<>>
struct result_type_declaration
{
    using result_type = typename functor_traits<T>::return_type;
};

/**
 * If it does then DO NOT declare it
 */
template<typename T>
struct result_type_declaration<T, cxx17::void_t<typename T::result_type>>
{ };

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

template <typename T> struct strict_visitor<T> : T, result_type_declaration<T>
{
    using T::operator();

    template <typename U>
    strict_visitor(U&& f)
        : T(std::forward<U>(f)) { }
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

template <typename T> struct weak_visitor<T> : T, result_type_declaration<T>
{
    using T::operator();
    /**
     * This is the only difference between 'strict_visitor' and 'weak_visitor'.
     * 'weak_visitor' will call this template if it obtains variant which doesn't match any of the provided functors.
     */
    template <typename U> void operator()(const U&) const {}

    template <typename U>
    weak_visitor(U&& f)
        : T(std::forward<U>(f)) { }
};
}

template <typename T, typename... Ts> auto make_strict_visitor(T&& f, Ts&&... fs)
    -> decltype(impl::strict_visitor<typename std::decay<T>::type, typename std::decay<Ts>::type...>(std::forward<T>(f), std::forward<Ts>(fs)...))
{
    return impl::strict_visitor<typename std::decay<T>::type, typename std::decay<Ts>::type...>(std::forward<T>(f), std::forward<Ts>(fs)...);
}

template <typename T, typename... Ts> auto make_weak_visitor(T&& f, Ts&&... fs)
    -> decltype(impl::weak_visitor<typename std::decay<T>::type, typename std::decay<Ts>::type...>(std::forward<T>(f), std::forward<Ts>(fs)...))
{
    return impl::weak_visitor<typename std::decay<T>::type, typename std::decay<Ts>::type...>(std::forward<T>(f), std::forward<Ts>(fs)...);
}
}