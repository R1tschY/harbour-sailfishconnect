/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ASCONST_H
#define ASCONST_H

#include <type_traits>
#include <memory>

namespace SailfishConnect {

template<typename T>
constexpr typename std::add_const<T>::type& asConst(T& t) noexcept
{
    return t;
}

template<typename T, typename...Args>
std::unique_ptr<T> makeUniquePtr(Args&&...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}


template<typename...Args>
struct NonConstOverload
{
    template<typename R, typename T>
    static constexpr auto of(R (T::*func)(Args...)) noexcept -> decltype(func)
    {
        return func;
    }
};

template<typename...Args>
struct ConstOverload
{
    template<typename R, typename T>
    static constexpr auto of(R (T::*func)(Args...) const) noexcept -> decltype(func)
    {
        return func;
    }
};

template<typename...Args>
struct Overload : ConstOverload<Args...>, NonConstOverload<Args...>
{
    using ConstOverload<Args...>::of;
    using NonConstOverload<Args...>::of;

    template<typename R>
    static constexpr auto of(R (*func)(Args...)) noexcept -> decltype(func)
    {
        return func;
    }
};

} // SailfishConnect

#endif // ASCONST_H
