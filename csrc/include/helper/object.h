#pragma once
#include <algorithm>
#include <exception>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
namespace interpreter {

// syntax: find T's index in Res
template <typename T, typename... Res>
struct TypeIndex;

// if match
template <typename T, typename... Res>
struct TypeIndex<T, T, Res...> : std::integral_constant<int, 0> {};

// if not match
template <typename T, typename U, typename... Res>
struct TypeIndex<T, U, Res...>
    : std::integral_constant<int, 1 + TypeIndex<T, Res...>::value> {};

template <typename... Ts>
struct Object {
 private:
  std::tuple<Ts...> TypeList;
  alignas(std::max({alignof(Ts)...})) char buf[std::max({sizeof(Ts)...})];
  size_t index_;

 public:
  // Object(Ts&&... ts) { TypeList = std::make_tuple(std::forward<Ts>(ts)...); }

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
  template <typename Alternative>
  Object(Alternative&& T_j) {
    update_value(std::forward<Alternative>(T_j));
  }

  template <size_t... I>
  void constexpr_get_update(std::index_sequence<I...> _,
                            std::variant<Ts...>& rhs) {
    ((I == rhs.index() ? (update_value(std::get<I>(rhs)), true) : false) ||
     ...);
  }
  Object(std::variant<Ts...>& rhs) {
    constexpr_get_update(std::index_sequence_for<Ts...>{}, rhs);
  }
  Object& operator=(std::variant<Ts...>& rhs) {
    constexpr_get_update(std::index_sequence_for<Ts...>{}, rhs);
    return *this;
  }

  template <typename T>
  void update_value(T&& val) {
    index_ = TypeIndex<std::decay_t<T>, Ts...>::value;
    construct(std::forward<T>(val));
    // using Type = T;
  }
  template <typename T>
  void construct(T&& val) {
    using T_ = std::decay_t<T>;
    new (buf) T_(val);
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

  template <typename T>
  constexpr T& get() {
    auto index = TypeIndex<std::decay_t<T>, Ts...>::value;
    if (index != index_) throw std::exception();
    auto& ret = *reinterpret_cast<T*>(buf);
    return ret;
  }

  std::string to_string() {
    return std::visit(
        [](auto&& args) -> std::string {
          std::string class_name = typeid(args).name();

          char address[20];
          std::sprintf(address, "%p", (void*)&args);
          return class_name + "@" + address;
        },
        *this);
  }
};

}  // namespace interpreter
   // in variant class declaration.