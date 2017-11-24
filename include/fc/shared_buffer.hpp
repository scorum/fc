#pragma once

#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>

#include <fc/io/raw_fwd.hpp>
#include <fc/variant.hpp>

namespace fc {

    namespace bip = boost::interprocess;

    template <typename T> 
    using allocator = bip::allocator<T, bip::managed_mapped_file::segment_manager>;

    template <typename T> 
    using shared_vector = std::vector<T, allocator<T>>;

    using shared_buffer = bip::vector<char, allocator<char>>;

namespace raw {

    template <typename T> inline void pack(shared_buffer& raw, const T& v)
    {
        auto size = pack_size(v);
        raw.resize(size);
        datastream<char*> ds(raw.data(), size);
        pack(ds, v);
    }

    template <typename T> inline void unpack(const shared_buffer& raw, T& v)
    {
        datastream<const char*> ds(raw.data(), raw.size());
        unpack(ds, v);
    }

    template <typename T> inline T unpack(const shared_buffer& raw)
    {
        T v;
        datastream<const char*> ds(raw.data(), raw.size());
        unpack(ds, v);
        return v;
    }

} // namespace raw
} // namespace fc

