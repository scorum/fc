#pragma once

#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>

#include <fc/io/raw_fwd.hpp>
#include <fc/variant.hpp>

namespace fc {

    namespace bip = boost::interprocess;

    /* 
    *  shared_string is a string type placeable in shared memory,
    *  courtesy of Boost.Interprocess.
    */
    using shared_string = bip::basic_string<char, std::char_traits<char>, bip::allocator<char, bip::managed_mapped_file::segment_manager>>;

    struct strcmp_less
    {
        bool operator()(const shared_string& a, const shared_string& b) const
        {
            return less(a.c_str(), b.c_str());
        }

        bool operator()(const shared_string& a, const std::string& b) const
        {
            return less(a.c_str(), b.c_str());
        }

        bool operator()(const std::string& a, const shared_string& b) const
        {
            return less(a.c_str(), b.c_str());
        }

    private:
        inline bool less(const char* a, const char* b) const
        {
            return std::strcmp(a, b) < 0;
        }
    };

    inline std::string to_string(const shared_string& str)
    {
        return std::string(str.begin(), str.end());
    }
    inline void from_string(shared_string& out, const string& in)
    {
        out.assign(in.begin(), in.end());
    }


    inline void to_variant(const shared_string& s, variant& var)
    {
        var = fc::string(to_string(s));
    }

    inline void from_variant(const variant& var, shared_string& s)
    {
        auto str = var.as_string();
        s.assign(str.begin(), str.end());
    }
}
