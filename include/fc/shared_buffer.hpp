#pragma once

#include <fc/io/raw_fwd.hpp>
#include <fc/variant.hpp>
#include <fc/shared_containers.hpp>

namespace fc {

    using shared_buffer = shared_vector<char>;

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


