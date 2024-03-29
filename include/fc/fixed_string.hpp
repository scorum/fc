#pragma once
#include <fc/uint128.hpp>
#include <fc/io/raw_fwd.hpp>

#include <boost/endian/conversion.hpp>

namespace fc {

/**
 * Endian-reversible pair class.
 */
template <typename A, typename B> struct erpair
{
    erpair()
    {
    }
    erpair(const A& a, const B& b)
        : first(a)
        , second(b)
    {
    }
    friend bool operator<(const erpair& a, const erpair& b)
    {
        return std::tie(a.first, a.second) < std::tie(b.first, b.second);
    }
    friend bool operator<=(const erpair& a, const erpair& b)
    {
        return std::tie(a.first, a.second) <= std::tie(b.first, b.second);
    }
    friend bool operator>(const erpair& a, const erpair& b)
    {
        return std::tie(a.first, a.second) > std::tie(b.first, b.second);
    }
    friend bool operator>=(const erpair& a, const erpair& b)
    {
        return std::tie(a.first, a.second) >= std::tie(b.first, b.second);
    }
    friend bool operator==(const erpair& a, const erpair& b)
    {
        return std::tie(a.first, a.second) == std::tie(b.first, b.second);
    }
    friend bool operator!=(const erpair& a, const erpair& b)
    {
        return std::tie(a.first, a.second) != std::tie(b.first, b.second);
    }

    A first{};
    B second{};
};

template <typename A, typename B> erpair<A, B> make_erpair(const A& a, const B& b)
{
    return erpair<A, B>(a, b);
}

template <typename T> T endian_reverse(const T& x)
{
    return boost::endian::endian_reverse(x);
}

template <> inline uint128 endian_reverse(const uint128& u)
{
    return uint128(boost::endian::endian_reverse(u.hi), boost::endian::endian_reverse(u.lo));
}

template <typename A, typename B> erpair<A, B> endian_reverse(const erpair<A, B>& p)
{
    return make_erpair(endian_reverse(p.first), endian_reverse(p.second));
}

/**
 *  This class is designed to offer in-place memory allocation of a string up to Length equal to
 *  sizeof(Storage).
 *
 *  The string will serialize the same way as std::string for variant and raw formats
 *  The string will sort according to the comparison operators defined for Storage, this enables efficient
 *  sorting.
 */
template <typename Storage = fc::uint128> class fixed_string
{
public:
    fixed_string()
    {
    }
    fixed_string(const fixed_string& c)
        : data(c.data)
    {
    }
    fixed_string(const char* str)
        : fixed_string(std::string(str))
    {
    }
    fixed_string(const std::string& str)
    {
        Storage d;
        if (str.size() <= sizeof(d))
            memcpy((char*)&d, str.c_str(), str.size());
        else
            memcpy((char*)&d, str.c_str(), sizeof(d));

        data = boost::endian::big_to_native(d);
    }

    operator std::string() const
    {
        Storage d = boost::endian::native_to_big(data);
        size_t s;

        if (*(((const char*)&d) + sizeof(d) - 1))
            s = sizeof(d);
        else
            s = strnlen((const char*)&d, sizeof(d));

        const char* self = (const char*)&d;

        return std::string(self, self + s);
    }

    uint32_t size() const
    {
        Storage d = boost::endian::native_to_big(data);
        if (*(((const char*)&d) + sizeof(d) - 1))
            return sizeof(d);
        return strnlen((const char*)&d, sizeof(d));
    }

    uint32_t length() const
    {
        return size();
    }

    fixed_string& operator=(const fixed_string& str)
    {
        data = str.data;
        return *this;
    }

    fixed_string& operator=(const char* str)
    {
        *this = fixed_string(str);
        return *this;
    }

    fixed_string& operator=(const std::string& str)
    {
        *this = fixed_string(str);
        return *this;
    }

    friend std::string operator+(const fixed_string& a, const std::string& b)
    {
        return std::string(a) + b;
    }
    friend std::string operator+(const std::string& a, const fixed_string& b)
    {
        return a + std::string(b);
    }
    friend bool operator<(const fixed_string& a, const fixed_string& b)
    {
        return a.data < b.data;
    }
    friend bool operator<=(const fixed_string& a, const fixed_string& b)
    {
        return a.data <= b.data;
    }
    friend bool operator>(const fixed_string& a, const fixed_string& b)
    {
        return a.data > b.data;
    }
    friend bool operator>=(const fixed_string& a, const fixed_string& b)
    {
        return a.data >= b.data;
    }
    friend bool operator==(const fixed_string& a, const fixed_string& b)
    {
        return a.data == b.data;
    }
    friend bool operator!=(const fixed_string& a, const fixed_string& b)
    {
        return a.data != b.data;
    }

    Storage data;
};

// These storage types work with memory layout and should be used instead of a custom template.
using fixed_string_16 = fixed_string<fc::uint128_t>;
using fixed_string_24 = fixed_string<fc::erpair<fc::uint128_t, uint64_t>>;
using fixed_string_32 = fixed_string<fc::erpair<fc::uint128_t, fc::uint128_t>>;
using uint256_t = fc::erpair<fc::uint128_t, fc::uint128_t>;
using uint512_t = fc::erpair<uint256_t, uint256_t>;
using fixed_utf8_string_24 = fixed_string<fc::erpair<uint256_t, uint512_t>>; // 768 bits = 4bytes x 24

namespace raw {

template <typename Stream, typename Storage> inline void pack(Stream& s, const fixed_string<Storage>& u)
{
    pack(s, std::string(u));
}

template <typename Stream, typename Storage> inline void unpack(Stream& s, fixed_string<Storage>& u)
{
    std::string str;
    unpack(s, str);
    u = str;
}

} // namespace raw

template <typename Storage> void to_variant(const fixed_string<Storage>& s, variant& v)
{
    v = std::string(s);
}

template <typename Storage> void from_variant(const variant& v, fixed_string<Storage>& s)
{
    s = v.as_string();
}

template <typename Stream, typename Storage> Stream& operator<<(Stream& stream, const fixed_string<Storage>& s)
{
    stream << static_cast<std::string>(s);
    return stream;
}

template <typename Stream, typename Storage> Stream& operator>>(Stream& stream, fixed_string<Storage>& s)
{
    std::string str;
    stream >> str;
    s = str;
    return stream;
}
} // namespace fc

namespace boost {
template <typename T> auto hash_value(const fc::fixed_string<T>& str)
{
    auto it = (const char*)&str.data;
    auto count = sizeof(str.data);

    size_t hash = 0;
    for (size_t i = 0; i < count; ++i, ++it)
    {
        hash_combine(hash, *it);
    }

    return hash;
}
}
