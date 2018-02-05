#pragma once

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

namespace fc {

template <typename T>
using shared_allocator = boost::interprocess::allocator<T, boost::interprocess::managed_mapped_file::segment_manager>;
}
