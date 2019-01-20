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
