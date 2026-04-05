#pragma once
#include <algorithm>
#include <exception>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
// clang-format off
/*
Lox type Lox类   Java representation Java表示
Any Lox value    Object
nil              null
Boolean          Boolean
number           Double
string           String
*/
// clang-format on
// syntax: find T's index in Res
template <typename T, typename... Res>
struct TypeIndex;

// if match
template <typename T, typename... Res>
struct TypeIndex<T, T, Res...> : std::integral_constant<size_t, 0> {};

// if not match
template <typename T, typename U, typename... Res>
struct TypeIndex<T, U, Res...>
    : std::integral_constant<size_t, 1 + TypeIndex<T, Res...>::value> {};

template <typename... Ts>
struct Object {
  Object() = default;
  // copy ctor
  Object(const Object<Ts...>& other) {
    constexpr_get_update(std::index_sequence_for<Ts...>{}, other);
  }

  // copy assignment
  Object& operator=(const Object<Ts...>& rhs) {
    destroy();  // since we have already value in buf, free it to avoid leakage.
    constexpr_get_update(std::index_sequence_for<Ts...>{}, rhs);
    return *this;
  }

  // move ctor
  Object(Object<Ts...>&& other) {
    constexpr_get_move(std::index_sequence_for<Ts...>{}, std::move(other));
  }

  // move assignment
  Object& operator=(Object<Ts...>&& other) {
    destroy();  // since we have already value in buf, free it to avoid leakage.
    constexpr_get_move(std::index_sequence_for<Ts...>{}, std::move(other));
    return *this;
  }

  // converting ctor
  Object(const std::variant<Ts...>& rhs) {
    constexpr_get_update(std::index_sequence_for<Ts...>{}, rhs);
  }

  template <size_t... I>
  bool isEqual(std::index_sequence<I...> _, Object& rhs) {
    // using T = std::tuple_element_t<I, std::tuple<Ts...>>;
    return (
        (index_ == I && hold_alternative<
                            std::tuple_element_t<I, std::tuple<Ts...>>>() ==
                            rhs.hold_alternative<
                                std::tuple_element_t<I, std::tuple<Ts...>>>()
             ? (get<std::tuple_element_t<I, std::tuple<Ts...>>>() ==
                    rhs.get<std::tuple_element_t<I, std::tuple<Ts...>>>() &&
                true)
             : false) ||
        ...);
  }

  bool operator==(Object& rhs) {
    return isEqual(std::index_sequence_for<Ts...>{}, rhs);
  }

  template <typename T>
  bool hold_alternative() {
    auto expected_index = TypeIndex<T, Ts...>::value;
    if (expected_index == index_) return true;
    return false;
  }
  constexpr auto index() const { return index_; }

  template <typename T>
  struct F_one {
    int operator()(T _);
  };

  template <typename... T>
  struct F : F_one<T>... {
    using F_one<T>::operator()...;
  };

  template <typename Alternative,
            typename Ti = decltype(F<Ts...>{}(std::declval<Alternative>()))>
  Object(Alternative&& T_j) {
    update_value(std::forward<Alternative>(T_j));
  }

  template <typename T>
  void construct(T&& val) {
    using T_ = std::decay_t<T>;
    new (buf) T_(std::forward<T>(val));
  }

  template <typename T>
  struct type_tag {
    using type = T;
  };

  template <size_t... I>
  void destroy_impl(std::integer_sequence<size_t, I...> _) {
    ((index_ == I
          ? (
                [&](auto tag) -> void {
                  using T = typename decltype(tag)::type;
                  reinterpret_cast<T*>(buf)->~T();
                }(type_tag<std::tuple_element_t<I, std::tuple<Ts...>>>{}),
                true)
          : false) ||
     ...);
  }

  void destroy() noexcept { destroy_impl(std::index_sequence_for<Ts...>{}); }

  // template <typename T>
  // constexpr T& get() {
  //   auto index = TypeIndex<std::decay_t<T>, Ts...>::value;
  //   if (index != index_) throw std::exception();
  //   auto& ret = *reinterpret_cast<T*>(buf);
  //   return ret;
  // }

  template <typename T>
  constexpr const T& get() const {
    auto index = TypeIndex<std::decay_t<T>, Ts...>::value;
    if (index != index_) throw std::exception();
    auto& ret = *reinterpret_cast<const T*>(buf);
    return ret;
  }

  template <typename T>
  constexpr T& get() {
    auto index = TypeIndex<std::decay_t<T>, Ts...>::value;
    if (index != index_) throw std::exception();
    auto& ret = *reinterpret_cast<T*>(buf);
    return ret;
  }

  ~Object() { destroy(); }

 private:
  std::tuple<Ts...> TypeList;
  alignas(std::max({alignof(Ts)...})) char buf[std::max({sizeof(Ts)...})];
  size_t index_;

  template <size_t... I>
  void constexpr_get_update(std::index_sequence<I...> _,
                            std::variant<Ts...>& rhs) {
    ((I == rhs.index() ? (update_value(std::get<I>(rhs)), true) : false) ||
     ...);
  }

  template <size_t... I>
  void constexpr_get_update(std::index_sequence<I...> _,
                            const Object<Ts...>& rhs) {
    ((I == rhs.index()
          ? (update_value(
                 rhs.get<std::tuple_element_t<I, std::tuple<Ts...>>>()),
             true)
          : false) ||
     ...);
  }

  template <typename T>
  void move_value(T&& val) {
    index_ = TypeIndex<std::decay_t<T>, Ts...>::value;
    construct(std::forward<T>(val));
  }

  template <typename T>
  void update_value(T&& val) {
    index_ = TypeIndex<std::decay_t<T>, Ts...>::value;
    construct(std::forward<T>(val));
  }

  template <size_t... I>
  void constexpr_get_move(std::index_sequence<I...> _, Object<Ts...>&& other) {
    (((I == other.index())
          ? (move_value(std::move(
                 other.get<std::tuple_element_t<I, std::tuple<Ts...>>>())),
             true)
          : false) ||
     ...);
  }
};
