/** This source adapted from https://github.com/kmicklas/variadic-static_variant
 *
 * Copyright (C) 2013 Kenneth Micklas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **/
#pragma once
#include <stdexcept>
#include <typeinfo>
#include <fc/exception/exception.hpp>
#include <fc/static_variant_visitor.hpp>

namespace fc {

// Implementation details, the user should not import this:
namespace impl {

template<int N, typename... Ts>
struct storage_ops;
template<int N, typename... Ts>
struct storage_ops_deduce_return;

template<typename X, typename... Ts>
struct position;

template<typename... Ts>
struct type_info;

template<typename StaticVariant>
struct copy_construct
{
   typedef void result_type;
   StaticVariant& sv;
   copy_construct( StaticVariant& s ):sv(s){}
   template<typename T>
   void operator()( const T& v )const
   {
      sv.init(v);
   }
};

template<typename StaticVariant>
struct move_construct
{
   typedef void result_type;
   StaticVariant& sv;
   move_construct( StaticVariant& s ):sv(s){}
   template<typename T>
   void operator()( T& v )const
   {
      sv.init( std::move(v) );
   }
};

template<int N, typename T, typename... Ts>
struct storage_ops<N, T&, Ts...> {
    static void del(int n, void *data) {}
    static void con(int n, void *data) {}

    template<typename TVisitor, typename TData>
    static typename TVisitor::result_type apply(int n, TData *data, TVisitor& v) {}
};

template<int N, typename T, typename... Ts>
struct storage_ops<N, T, Ts...> {
    static void del(int n, void *data) {
        if(n == N) reinterpret_cast<T*>(data)->~T();
        else storage_ops<N + 1, Ts...>::del(n, data);
    }
    static void con(int n, void *data) {
        if(n == N) new(reinterpret_cast<T*>(data)) T();
        else storage_ops<N + 1, Ts...>::con(n, data);
    }

    template<typename TVisitor, typename TData>
    static typename TVisitor::result_type apply(int n, TData *data, TVisitor& v) {
        using data_type = std::conditional_t<std::is_const<TData>::value, const T*, T*>;

        if(n == N) return v(*reinterpret_cast<data_type>(data));
        else return storage_ops<N + 1, Ts...>::apply(n, data, v);
    }
};

template<int N>
struct storage_ops<N> {
    static void del(int n, void *data) {
       FC_THROW_EXCEPTION( fc::assert_exception, "Internal error: static_variant tag is invalid.");
    }
    static void con(int n, void *data) {
       FC_THROW_EXCEPTION( fc::assert_exception, "Internal error: static_variant tag is invalid." );
    }

    template<typename TVisitor, typename TData>
    static typename TVisitor::result_type apply(int n, TData *data, TVisitor& v) {
        FC_THROW_EXCEPTION( fc::assert_exception, "Internal error: static_variant tag is invalid." );
    }
};

template<int N, typename T, typename... Ts>
struct storage_ops_deduce_return<N, T&, Ts...> {
    static void del(int n, void *data) {}
    static void con(int n, void *data) {}

    template<typename TVisitor, typename TData>
    static void apply(int n, TData *data, TVisitor& v) {}
};

template<int N, typename T, typename... Ts>
struct storage_ops_deduce_return<N, T, Ts...> {
    static void del(int n, void *data) {
        if(n == N) reinterpret_cast<T*>(data)->~T();
        else storage_ops_deduce_return<N + 1, Ts...>::del(n, data);
    }
    static void con(int n, void *data) {
        if(n == N) new(reinterpret_cast<T*>(data)) T();
        else storage_ops_deduce_return<N + 1, Ts...>::con(n, data);
    }

    template<typename TVisitor, typename TData>
    static decltype(auto) apply(int n, TData *data, TVisitor& v) {
        using data_type = std::conditional_t<std::is_const<TData>::value, const T*, T*>;

        if(n == N) return v(*reinterpret_cast<data_type>(data));
        else return storage_ops_deduce_return<N + 1, Ts...>::apply(n, data, v);
    }
};

template<int N, typename T>
struct storage_ops_deduce_return<N, T> {
    static void del(int n, void *data) {
        if(n == N) reinterpret_cast<T*>(data)->~T();
        else FC_THROW_EXCEPTION( fc::assert_exception, "Internal error: static_variant tag is invalid.");
    }
    static void con(int n, void *data) {
        if(n == N) new(reinterpret_cast<T*>(data)) T();
        else FC_THROW_EXCEPTION( fc::assert_exception, "Internal error: static_variant tag is invalid.");
    }

    template<typename TVisitor, typename TData>
    static decltype(auto) apply(int n, TData *data, TVisitor& v) {
        using data_type = std::conditional_t<std::is_const<TData>::value, const T*, T*>;

        if(n == N) return v(*reinterpret_cast<data_type>(data));
        else FC_THROW_EXCEPTION( fc::assert_exception, "Internal error: static_variant tag is invalid.");
    }
};

template<typename X>
struct position<X> {
    static const int pos = -1;
};

template<typename X, typename... Ts>
struct position<X, X, Ts...> {
    static const int pos = 0;
};

template<typename X, typename T, typename... Ts>
struct position<X, T, Ts...> {
    static const int pos = position<X, Ts...>::pos != -1 ? position<X, Ts...>::pos + 1 : -1;
};

template<typename T, typename... Ts>
struct type_info<T&, Ts...> {
    static const bool no_reference_types = false;
    static const bool no_duplicates = position<T, Ts...>::pos == -1 && type_info<Ts...>::no_duplicates;
    static const size_t size = type_info<Ts...>::size > sizeof(T&) ? type_info<Ts...>::size : sizeof(T&);
    static const size_t count = 1 + type_info<Ts...>::count;
};

template<typename T, typename... Ts>
struct type_info<T, Ts...> {
    static const bool no_reference_types = type_info<Ts...>::no_reference_types;
    static const bool no_duplicates = position<T, Ts...>::pos == -1 && type_info<Ts...>::no_duplicates;
    static const size_t size = type_info<Ts...>::size > sizeof(T) ? type_info<Ts...>::size : sizeof(T&);
    static const size_t count = 1 + type_info<Ts...>::count;
};

template<>
struct type_info<> {
    static const bool no_reference_types = true;
    static const bool no_duplicates = true;
    static const size_t count = 0;
    static const size_t size = 0;
};

} // namespace impl

template<typename... Types>
class static_variant {
    static_assert(impl::type_info<Types...>::no_reference_types, "Reference types are not permitted in static_variant.");
    static_assert(impl::type_info<Types...>::no_duplicates, "static_variant type arguments contain duplicate types.");

    int _tag;
    char storage[impl::type_info<Types...>::size];

    template<typename X>
    void init(const X& x) {
        _tag = impl::position<X, Types...>::pos;
        new(storage) X(x);
    }

    template<typename X>
    void init(X&& x) {
        _tag = impl::position<X, Types...>::pos;
        new(storage) X( std::move(x) );
    }

    template<typename StaticVariant>
    friend struct impl::copy_construct;
    template<typename StaticVariant>
    friend struct impl::move_construct;
public:
    template<typename X>
    struct tag
    {
       static_assert(
         impl::position<X, Types...>::pos != -1,
         "Type not in static_variant."
       );
       static const int value = impl::position<X, Types...>::pos;
    };
    static_variant()
    {
       _tag = 0;
       impl::storage_ops<0, Types...>::con(0, storage);
    }

    template<typename... Other>
    static_variant( const static_variant<Other...>& cpy )
    {
       cpy.visit( impl::copy_construct<static_variant>(*this) );
    }
    static_variant( const static_variant& cpy )
    {
       cpy.visit( impl::copy_construct<static_variant>(*this) );
    }

    static_variant( static_variant&& mv )
    {
       mv.visit( impl::move_construct<static_variant>(*this) );
    }

    template<typename X>
    static_variant(const X& v) {
        static_assert(
            impl::position<X, Types...>::pos != -1,
            "Type not in static_variant."
        );
        init(v);
    }
    ~static_variant() {
       impl::storage_ops<0, Types...>::del(_tag, storage);
    }


    template<typename X>
    static_variant& operator=(const X& v) {
        static_assert(
            impl::position<X, Types...>::pos != -1,
            "Type not in static_variant."
        );
        this->~static_variant();
        init(v);
        return *this;
    }
    static_variant& operator=( const static_variant& v )
    {
       if( this == &v ) return *this;
       this->~static_variant();
       v.visit( impl::copy_construct<static_variant>(*this) );
       return *this;
    }
    static_variant& operator=( static_variant&& v )
    {
       if( this == &v ) return *this;
       this->~static_variant();
       v.visit( impl::move_construct<static_variant>(*this) );
       return *this;
    }
    friend bool operator == ( const static_variant& a, const static_variant& b )
    {
       return a.equal_to(b);
    }
    friend bool operator < ( const static_variant& a, const static_variant& b )
    {
       return a.less_than(b);
    }

    // Use template specialization to overload these comparison functions
    bool less_than(const static_variant& b) const
    {
        return which() < b.which();
    }

    bool equal_to(const static_variant& b) const
    {
        return which() == b.which();
    }

    template<typename X>
    X& get() {
        static_assert(
            impl::position<X, Types...>::pos != -1,
            "Type not in static_variant."
        );
        if(_tag == impl::position<X, Types...>::pos) {
            void* tmp(storage);
            return *reinterpret_cast<X*>(tmp);
        } else {
            FC_THROW_EXCEPTION( fc::assert_exception, "static_variant does not contain a value of type ${t}", ("t",fc::get_typename<X>::name()) );
           //     std::string("static_variant does not contain value of type ") + typeid(X).name()
           // );
        }
    }
    template<typename X>
    const X& get() const {
        static_assert(
            impl::position<X, Types...>::pos != -1,
            "Type not in static_variant."
        );
        if(_tag == impl::position<X, Types...>::pos) {
            const void* tmp(storage);
            return *reinterpret_cast<const X*>(tmp);
        } else {
            FC_THROW_EXCEPTION( fc::assert_exception, "static_variant does not contain a value of type ${t}", ("t",fc::get_typename<X>::name()) );
        }
    }

    /**
     * These overloads should be used when you are passing functor object with all required operator()(...)
     *
     * NOTE: No copy/move will be performed
     */
    template<typename visitor>
    decltype(auto) visit(visitor&& v)const {
        return impl::storage_ops_deduce_return<0, Types...>::apply(_tag, storage, v);
    }

    template<typename visitor>
    decltype(auto) visit(visitor&& v) {
        return impl::storage_ops_deduce_return<0, Types...>::apply(_tag, storage, v);
    }

    /**
     * You can pass multiple functors where each one handle particular variant in fc::static_variant
     *
     * NOTE: All these functors will be copied/moved in order to construct visitor.
     */
    template<typename TF, typename... TFs>
    decltype(auto) visit(TF&& f, TFs&&... fs)
    {
        auto v = make_strict_visitor(std::forward<TF>(f), std::forward<TFs>(fs)...);
        return impl::storage_ops_deduce_return<0, Types...>::apply(_tag, storage, v);
    };

    /**
     * You can pass multiple functors where each one handle particular variant in fc::static_variant
     *
     * NOTE: All these functors will be copied/moved in order to construct visitor.
     */
    template<typename TF, typename... TFs>
    decltype(auto) visit(TF&& f, TFs&&... fs) const
    {
        auto v = make_strict_visitor(std::forward<TF>(f), std::forward<TFs>(fs)...);
        return impl::storage_ops_deduce_return<0, Types...>::apply(_tag, storage, v);
    };

    /**
     * Created visitor can match only the subset of 'fc::static_variant's variants. The rest will be ignored.
     *
     * NOTE: All these functors will be copied/moved in order to construct visitor.
     */
    template<typename TF, typename... TFs>
    auto weak_visit(TF&& functor, TFs&&... functors)
    {
        auto v = make_weak_visitor(std::forward<TF>(functor), std::forward<TFs>(functors)...);
        return impl::storage_ops<0, Types...>::apply(_tag, storage, v);
    };

    /**
     * Created visitor can match only the subset of 'fc::static_variant's variants. The rest will be ignored.
     *
     * NOTE: All these functors will be copied/moved in order to construct visitor.
     */
    template<typename TF, typename... TFs>
    auto weak_visit(TF&& functor, TFs&&... functors) const
    {
        auto v = make_weak_visitor(std::forward<TF>(functor), std::forward<TFs>(functors)...);
        return impl::storage_ops<0, Types...>::apply(_tag, storage, v);
    };

    static int count() { return impl::type_info<Types...>::count; }
    void set_which( int w ) {
      FC_ASSERT( w < count() );
      this->~static_variant();
      _tag = w;
      impl::storage_ops<0, Types...>::con(_tag, storage);
    }

    int which() const {return _tag;}
};

template<typename Result>
struct visitor {
    typedef Result result_type;
};

   struct from_static_variant 
   {
      variant& var;
      from_static_variant( variant& dv ):var(dv){}

      typedef void result_type;
      template<typename T> void operator()( const T& v )const
      {
         to_variant( v, var );
      }
   };

   struct to_static_variant
   {
      const variant& var;
      to_static_variant( const variant& dv ):var(dv){}

      typedef void result_type;
      template<typename T> void operator()( T& v )const
      {
         from_variant( var, v ); 
      }
   };


   template<typename... T> void to_variant( const fc::static_variant<T...>& s, fc::variant& v )
   {
      variant tmp;
      variants vars(2);
      vars[0] = s.which();
      s.visit( from_static_variant(vars[1]) );
      v = std::move(vars);
   }
   template<typename... T> void from_variant( const fc::variant& v, fc::static_variant<T...>& s )
   {
      auto ar = v.get_array();
      if( ar.size() < 2 ) return;
      s.set_which( ar[0].as_uint64() );
      s.visit( to_static_variant(ar[1]) );
   }

} // namespace fc
