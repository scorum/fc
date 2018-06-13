#pragma once

namespace fc {
namespace impl {
/**
 * This won't compile if where are variants which weren't matched.
 * Functor or lambda is just a struct with operator(). So build a visitor by inheriting from all such functors.
 * As a result we will get a struct with each required operator()(...)
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
    using result_type = void;
    using T::operator();

    template <typename U>
    strict_visitor(U&& f)
        : T(std::forward<U>(f)) { }
};

/**
 * This will ignore variants which weren't matched.
 * Functor or lambda is just a struct with operator(). So build a visitor by inheriting from all such functors.
 * As a result we will get a struct with each required operator()(...)
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

template <typename T> struct weak_visitor<T> : T
{
    using result_type = void;
    using T::operator();
    template <typename U> void operator()(const U&) const {}

    template <typename U>
    weak_visitor(U&& f)
        : T(std::forward<U>(f)) { }
};
}

template <typename T, typename... Ts> auto make_strict_visitor(T&& f, Ts&&... fs)
    -> decltype(impl::strict_visitor<T, Ts...>(std::forward<T>(f), std::forward<Ts>(fs)...))
{
    return impl::strict_visitor<T, Ts...>(std::forward<T>(f), std::forward<Ts>(fs)...);
}

template <typename T, typename... Ts> auto make_weak_visitor(T&& f, Ts&&... fs)
    -> decltype(impl::weak_visitor<T, Ts...>(std::forward<T>(f), std::forward<Ts>(fs)...))
{
    return impl::weak_visitor<T, Ts...>(std::forward<T>(f), std::forward<Ts>(fs)...);
}
}