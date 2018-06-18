#pragma once
#include <type_traits>

namespace fc
{
/**
 * If return type is NOT void
 */
template<typename TF, typename TRet, typename TArg>
typename std::enable_if<!std::is_void<TRet>::value, TRet>::type get_ret_type(TRet(TF::*)(TArg)const) { return TRet{}; }

template<typename TF, typename TRet, typename TArg>
typename std::enable_if<!std::is_void<TRet>::value, TRet>::type get_ret_type(TRet(TF::*)(TArg&)const) { return TRet{}; }

template<typename TF, typename TRet, typename TArg>
typename std::enable_if<!std::is_void<TRet>::value, TRet>::type get_ret_type(TRet(TF::*)(TArg&&)const) { return TRet{}; }

/**
 * If return type is void
 */
template<typename TF, typename TRet, typename TArg>
typename std::enable_if<std::is_void<TRet>::value, TRet>::type get_ret_type(TRet(TF::*)(TArg)const) { }

template<typename TF, typename TRet, typename TArg>
typename std::enable_if<std::is_void<TRet>::value, TRet>::type get_ret_type(TRet(TF::*)(TArg&)const) { }

template<typename TF, typename TRet, typename TArg>
typename std::enable_if<std::is_void<TRet>::value, TRet>::type get_ret_type(TRet(TF::*)(TArg&&)const) { }

template<typename TF>
struct functor_traits
{
    using return_type = decltype(get_ret_type(&TF::operator()));
};
}