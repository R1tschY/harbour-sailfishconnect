#ifndef ASCONST_H
#define ASCONST_H

#include <type_traits>

namespace SailfishConnect {

template <class T>
constexpr typename std::add_const<T>::type& asConst(T& t) noexcept
{
    return t;
}

} // SailfishConnect

#endif // ASCONST_H
