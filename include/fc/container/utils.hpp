#pragma once

#include <iterator>

namespace fc {

template <typename T> void limit_left_cut_right(T& container, std::size_t limit)
{
    if (container.size() > limit)
    {
        auto end_iter = container.begin();
        std::advance(end_iter, limit);
        container.erase(end_iter, container.end());
    }
}

} // namespace fc
