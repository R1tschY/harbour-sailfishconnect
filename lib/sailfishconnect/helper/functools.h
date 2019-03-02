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

#ifndef FUNCTOOLS_H
#define FUNCTOOLS_H

namespace SailfishConnect {

// template<typename F>
// auto byKey(F&& f) {
//     return [f](const auto& t1, const auto& t2) { return f(t1) < f(t2); };
// }

namespace details {

template<typename F>
class byKey {
public:
    F f;

    byKey(F&& f) : f(std::move(f)) { }

    template<typename T>
    bool operator()(const T& t1, const T& t2) {
        return f(t1) < f(t2);
    }
};

template<typename F>
class byKeyDesc {
public:
    F f;

    byKeyDesc(F&& f) : f(std::move(f)) { }

    template<typename T>
    bool operator()(const T& t1, const T& t2) {
        return f(t1) > f(t2);
    }
};

} // details

template<typename F>
details::byKey<F> byKey(F&& f) {
  return details::byKey<F>(std::move(f));
}

template<typename F>
details::byKeyDesc<F> byKeyDesc(F&& f) {
  return details::byKeyDesc<F>(std::move(f));
}

} // SailfishConnect

#endif // FUNCTOOLS_H
