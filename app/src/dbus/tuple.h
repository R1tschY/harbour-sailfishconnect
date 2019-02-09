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
