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

template <class>
constexpr int length = 0;

template <class... t>
constexpr int length<_list<t...>> = sizeof...(t);

template <int... n>
using List = _list<Int<n>...>;

template <int...>
struct Indexer;

template <>
struct Indexer<> {
  template <class A>
  static auto& idx(A&& a) {
    return a;
  }
};

template <class...>
struct _cat;

template <>
struct _cat<> {
  using List = _list<>;
};

template <class... A, class... B>
struct _cat<_list<A...>, _list<B...>> {
  using List = _list<A..., B...>;
};

template <class... A>
using Cat = typename _cat<A...>::List;

template <template <int...> class T, class... A>
using CatMap = typename Cat<A...>::template Map<T>;

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

template <int n, template <int...> class T>
using SeqMap = typename Seq<n>::template Map<T>;

template <class>
struct pairer;

template <int... t>
struct Pair {
  template <int n>
  using PushFront = Pair<n, t...>;

  using Indexer = Indexer<t...>;
};

template <int... n>
using Paired = _list<Pair<n>...>;

template <class... p>
struct pairer<_list<p...>> {
  template <int a>
  using Unpacked = _list<typename p::template PushFront<a>...>;

  template <int... c>
  using Catted = Cat<Unpacked<c>...>;
};

template <int a, int... t>
struct _cartesian {
  template <int...>
  struct Rec;

  template <int h, int... c>
  struct Rec<h, c...> {
    using ty = SeqMap<h, pairer<typename Rec<c...>::ty>::template Catted>;
  };

  template <>
  struct Rec<> {
    using ty = SeqMap<a, Paired>;
  };

  using ty = typename Rec<t...>::ty;
};

template <int... n>
using Cartesian = typename _cartesian<n...>::ty;

template <int h, int... n>
struct Indexer<h, n...> {
  template <class A>
  static auto& idx(A&& a) {
    if constexpr (sizeof...(n) != 0)
      return Indexer<n...>::idx(a[h]);
    else
      return a[h];
  }
};

template <int n, class...>
struct Split;

template <int n, class...>
struct SplitList;

template <int n, class... l, class... r>
struct SplitList<n, _list<>, _list<l...>, _list<r...>> {
  using L = _list<typename l::Indexer...>;
  using R = _list<typename r::Indexer...>;
};

template <int n, class s0, class... s, class... l, class... r>
struct SplitList<n, _list<s0, s...>, _list<l...>, _list<r...>>
    : SplitList<n,
                _list<s...>,
                _list<l..., typename Split<n, s0, Pair<>>::L>,
                _list<r..., typename Split<n, s0, Pair<>>::R>> {};

template <int... l, int... r>
struct Split<0, Pair<l...>, Pair<r...>> {
  using L = Pair<l...>;
  using R = Pair<r...>;
};

template <int n, int... l, int... r>
struct Split<n, Pair<l...>, Pair<r...>>
    : Split<n - 1,
            typename Head<l...>::template Map<Pair>,
            Pair<last<l...>, r...>> {};

template <int, class, class>
struct Dotter;

template <int n, class... I, class... J>
struct Dotter<n, _list<I...>, _list<J...>> {
  template <int... k>
  struct Sequenced {
    template <class i, class j>
    struct Eval {
      template <class A, class B, class C>
      constexpr Eval(A const& a, B const& b, C& c) {
        j::idx(i::idx(c)) = ((i::idx(a)[k] * j::idx(b[k])) + ...);
      }
    };
  };

  template <class i, class j>
  using Eval = typename SeqMap<n, Sequenced>::template Eval<i, j>;

  template <class A, class B, class C>
  static C& dot(A&& a, B&& b, C& c) {
    (Eval<I, J>(a, b, c), ...);
    return c;
  }
};

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

  next& operator[](int i) { return dat[i]; }
  next const& operator[](int i) const { return dat[i]; }

  template <int... r>
  auto operator*(tensor<r...> const& rhs) const {
    if constexpr (sz == head<r...>) {
      return mul(rhs);
    }

    if constexpr (sz == 1) {
      return *this * (tensor<1, r...> const&)rhs;
    }

    if constexpr (head<r...> == 1) {
      return (tensor<t..., 1> const&)*this * rhs;
    }
  }

  template <int... r>
  auto mul(tensor<sz, r...> const& rhs) const {
    using Cart = CatMap<Cartesian, Head<List>, List<r...>>;
    using Split = SplitList<sizeof...(r), Cart, List<>, List<>>;

    using left = typename Split::L;
    using right = typename Split::R;

    static_assert(length<left> > 0);
    static_assert(length<right> > 0);
    static_assert(length<left> == length<right>);

    typename next::template cat<r...> out;
    return Dotter<sz, left, right>::dot(*this, rhs, out);
  }
};

template <int h>
struct tensor<h> {
  template <int... m>
  using cat = tensor<h, m...>;
  static constexpr int dim[] = {h};
  f32 dat[h];
  f32& operator[](int i) { return dat[i]; }
  f32 const& operator[](int i) const { return dat[i]; }
};

#include <regex>

using vec4 = tensor<4>;
using mat4 = tensor<4, 4>;

int main() {
  vec4 v = {1, 2, 3, 4};
  mat4 m = {v, v, v, v};

  auto q = m * v;

  printf("%.2f %.2f %.2f %.2f\n", q[0], q[1], q[2], q[3]);
  printf("%.2f %.2f %.2f %.2f\n", m[0][0], m[0][1], m[0][2], m[0][3]);
  printf("%.2f %.2f %.2f %.2f\n", m[1][0], m[1][1], m[1][2], m[1][3]);
  printf("%.2f %.2f %.2f %.2f\n", m[2][0], m[2][1], m[2][2], m[2][3]);
  printf("%.2f %.2f %.2f %.2f\n", m[3][0], m[3][1], m[3][2], m[3][3]);
}