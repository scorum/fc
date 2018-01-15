#pragma once

#include <boost/preprocessor/cat.hpp>
#include <boost/config.hpp>

#define SCOPED_INCREMENT(x) BOOST_ATTRIBUTE_UNUSED fc::incrementer<decltype(x)> BOOST_PP_CAT(x, _incrementer)(x);

namespace fc {

/**
* The code we want to implement is this:
*
* ++target; try { ... } finally { --target }
*
* In C++ the only way to implement finally is to create a class
* with a destructor, so that's what we do here.
*/
template <class T> class incrementer
{
public:
    incrementer(T& target)
        : _target(target)
    {
        ++_target;
    }
    ~incrementer()
    {
        --_target;
    }

private:
    T& _target;
};
}
