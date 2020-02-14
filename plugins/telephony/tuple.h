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

#ifndef CPPINTEGRATION_H
#define CPPINTEGRATION_H

#include <tuple>

#include <QDBusArgument>


namespace SailfishConnect {
namespace Dbus {
namespace _Details {

template<std::size_t I, typename T>
struct DbusTuple {
    static void unpack(QDBusArgument& arg, const T& src)
    {
        DbusTuple<I - 1, T>::unpack(arg, src);
        arg << std::get<I - 1>(src);
    }

    static void pack(const QDBusArgument& arg, T& dest)
    {
        DbusTuple<I - 1, T>::pack(arg, dest);
        arg >> std::get<I - 1>(dest);
    }
};

template<typename T>
struct DbusTuple<0, T> {
    static void unpack(QDBusArgument&, const T&) { }
    static void pack(const QDBusArgument&, T&) { }
};

} // namespace _Details
} // namespace Dbus
} // namespace SailfishConnect

template<typename...Args>
QDBusArgument& operator<<(QDBusArgument &arg, const std::tuple<Args...>& src)
{
    using namespace SailfishConnect::Dbus::_Details;

    arg.beginStructure();
    DbusTuple<sizeof...(Args), decltype(src)>::unpack(arg, src);
    arg.endStructure();
    return arg;
}

template<typename...Args>
const QDBusArgument& operator>>(
        const QDBusArgument& arg, std::tuple<Args...>& dest)
{
    using namespace SailfishConnect::Dbus::_Details;

    arg.beginStructure();
    DbusTuple<sizeof...(Args), decltype(dest)>::pack(arg, dest);
    arg.endStructure();
    return arg;
}

#endif // CPPINTEGRATION_H
