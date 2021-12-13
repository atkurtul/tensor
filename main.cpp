#include <stdio.h>
#include <typeinfo>

using f32 = float;

template <int, int... n>
static constexpr int last = last<n...>;

template <int n>
static constexpr int last<n> = n;

template <int n, int...>
static constexpr int head = n;

template <int n>
static constexpr int head<n> = n;

template <int>
struct Int;

template <class...>
struct _list {};

template <int... n>
using List = _list<Int<n>...>;

template <class...>
struct _cat;

template <class... A, class... B>
struct _cat<_list<A...>, _list<B...>> {
  using L = _list<A..., B...>;
};

template <class... A>
using Cat = typename _cat<A...>::L;

template <class... A, class... B>
struct _cat<_list<A...>, B...> : _cat<_list<A...>, Cat<B...>> {};

template <class...>
struct _rev;

template <int... n>
using Rev = _rev<_list<Int<n>...>, _list<>>;

template <int... n>
struct _list<Int<n>...> {
  template <template <int...> class T>
  using Map = T<n...>;

  using Rev = Rev<n...>;
};

template <class h, class... a, class... b>
struct _rev<_list<h, a...>, _list<b...>> : _rev<_list<a...>, _list<h, b...>> {};

template <int... n>
struct _rev<_list<>, _list<Int<n>...>> : _list<Int<n>...> {};

template <int...>
struct Tail;

template <int h, int... n>
struct Tail<h, n...> {
  template <template <int...> class T>
  using Map = T<n...>;
};

template <int... n>
struct Head {
  template <template <int...> class T>
  using Map = typename Rev<n...>::template Map<Tail>::template Map<
      Rev>::template Map<T>;
};

template <int... t>
struct _seq : _seq<head<t...> - 1, t...> {};

template <int... t>
struct _seq<-1, t...> {
  template <template <int...> class T>
  using Map = T<t...>;
};

template <int n>
using Seq = _seq<n - 1>;

template <class...>
struct _cart {};
template <int... n>
using Cart = _cart<typename Seq<n>::template Map<List>...>;

template <class>
struct pairer;

template <int... t>
struct Pair {
  template <int n>
  using PushFront = Pair<n, t...>;
};

template <int... n>
using Pairer = pairer<List<n...>>;

template <int... n>
struct pairer<List<n...>> {
  template <int a>
  using Unpacked = _list<Pair<a, n>...>;

  template <int... c>
  using Catted = Cat<Unpacked<c>...>;
};

template <class... p>
struct pairer<_list<p...>> {
  template <int a>
  using Unpacked = _list<typename p::template PushFront<a>...>;

  template <int... c>
  using Catted = Cat<Unpacked<c>...>;
};

template <int a, int b, int... t>
struct Cartesian {
  template <int...>
  struct Rec;

  template <int h, int... c>
  struct Rec<h, c...> {
    using ty = typename Seq<h>::template Map<
        pairer<typename Rec<c...>::ty>::template Catted>;
  };

  template <>
  struct Rec<> {
    using ty = typename Seq<b>::template Map<
        Seq<a>::template Map<Pairer>::template Catted>;
  };

  using ty = typename Rec<t...>::ty;
};

template <class>
struct Indexer;

template <int... n>
struct Indexer<Pair<n...>> {};

template <int... t>
struct tensor {
  template <int... m>
  using cat = tensor<t..., m...>;

  template <template <int...> class T>
  using Head = typename Head<t...>::template Map<T>;

  template <template <int...> class T>
  using Rev = typename Rev<t...>::template Map<T>;

  using next = Head<tensor>;
  using tpos_t = Rev<tensor>;

  static constexpr int dim[] = {t...};
  static constexpr int sz = last<t...>;

  next dat[sz];

  tpos_t tpos() const {}

  template <int... r>
  typename next::template cat<r...> operator*(tensor<sz, r...>) const {
    typename Cat<Head<List>, List<r...>>::template Map<tensor> x;
    return x;
  }
};

template <int h>
struct tensor<h> {
  template <int... m>
  using cat = tensor<h, m...>;

  static constexpr int dim[] = {h};
  f32 dat[h];
};

#include <regex>

int main() {
  // Cart<1, 2, 3> c;

  tensor<3, 3, 3>{} * tensor<3, 4, 5>{};

  std::string name = typeid(Cartesian<2, 2>::ty).name();

  std::regex re("<\\d+(,\\d+)*>");

  auto it = std::sregex_iterator(name.begin(), name.end(), re);

  for (; it != std::sregex_iterator(); ++it)
    printf("%s\n", it->str().c_str());

  // printf("%s\n", typeid(Cartesian<3,3,3>::ty).name());
}
