#pragma once

#include <fc/shared_allocator.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/container/map.hpp>
#include <boost/container/set.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
#include <boost/multi_index_container.hpp>

namespace fc {

template <typename T> using shared_vector = boost::container::vector<T, shared_allocator<T>>;

template <typename T> using shared_deque = boost::container::deque<T, shared_allocator<T>>;

template <typename Key> using shared_set = boost::container::set<Key, std::less<Key>, shared_allocator<Key>>;

template <class Key, class T>
using shared_map = boost::container::map<Key, T, std::less<Key>, shared_allocator<std::pair<const Key, T>>>;

template <class Key, class T>
using shared_flat_map = boost::container::flat_map<Key, T, std::less<Key>, shared_allocator<std::pair<Key, T>>>;

template <class Key, class Allocator = shared_allocator<Key>>
using shared_flat_set = boost::container::flat_set<Key, std::less<Key>, Allocator>;

template <typename Object, typename... Args>
using shared_multi_index_container = boost::multi_index_container<Object, Args..., shared_allocator<Object>>;
}
