#include<iostream>
#include<variant>
#include<type_traits>

namespace impl {

template<typename T, typename...Ts>
struct first_of{
    using type = T;
};

template<typename T, typename...Ts>
struct last_of{
    using type = typename last_of<Ts...>::type;
};

template<typename T>
struct last_of<T>{
    using type = T;
};

template<size_t N, typename T, typename...Ts>
struct at_index{
    using type = typename at_index<N-1, Ts...>::type;
};

template<typename T, typename...Ts>
struct at_index<0, T, Ts...>{
    using type = T;
};

template<typename T>
struct at_index<0,T>{
    using type = T;
};

template<size_t N, typename...Ts>
using at_index_t = typename at_index<N, Ts...>::type;

template<typename...Ts>
struct type_list{
    static std::tuple<std::remove_reference_t<Ts>...> types;
    static constexpr int size = sizeof...(Ts);
    using first = typename first_of<Ts...>::type;
    using last = typename last_of<Ts...>::type;
    template<size_t N, typename=std::enable_if_t<(N < size && N > 0)>>
    using at = at_index_t<N, Ts...>;
};

template<typename T>
struct deconstruct{
    template<template<typename...> typename TT, typename...Ts>
    static constexpr type_list<Ts...> get_list(TT<Ts...>*);
    using types = decltype(get_list(std::declval<T*>()));
};

template<typename Has, typename T, typename...Ts>
struct contains{
    using type = std::conditional_t<std::is_same_v<Has,T>, std::true_type, typename contains<Has,Ts...>::type>;
    static constexpr bool value = type::value;
};

template<typename Has, typename T>
struct contains<Has,T>{
    using type = std::conditional_t<std::is_same_v<Has,T>, std::true_type, std::false_type>;
    static constexpr bool value = type::value;
};

template<typename Has, typename T>
struct template_has{
    template<template<typename...> typename TT, typename...Ts>
    static constexpr contains<Has, Ts...> get_list(TT<Ts...>*);
    using type = decltype(get_list(std::declval<T*>()));
    static constexpr bool value = type::value;
};

template<typename Has, typename T>
constexpr bool template_has_v = template_has<Has,T>::value;

template<typename Has, typename T, typename...Ts>
struct contains_or_can_construct{
    static constexpr bool check_value = std::is_same_v<Has,T> || std::is_constructible_v<Has,T>;
    using type = std::conditional_t<check_value, std::true_type, typename contains<Has,Ts...>::type>;
    static constexpr bool value = type::value;
};

template<typename Has, typename T>
struct contains_or_can_construct<Has,T>{
    static constexpr bool check_value = std::is_same_v<Has,T> || std::is_constructible_v<Has,T>;
    using type = std::conditional_t<check_value, std::true_type, std::false_type>;
    static constexpr bool value = type::value;
};

template<typename Has, typename T>
struct template_has_or_can_construct{
    template<template<typename...> typename TT, typename...Ts>
    static constexpr contains_or_can_construct<Has, Ts...> get_list(TT<Ts...>*);
    using type = decltype(get_list(std::declval<T*>()));
    static constexpr bool value = type::value;
};

template<typename Has, typename T>
constexpr bool template_has_or_can_construct_v = template_has_or_can_construct<Has,T>::value;

template<typename Has, typename T, typename...Ts>
struct can_only_construct{
    static constexpr bool check_value = !std::is_same_v<Has,T> && std::is_constructible_v<Has,T>;
    using type = std::conditional_t<check_value, std::true_type, typename contains<Has,Ts...>::type>;
    static constexpr bool value = type::value;
};

template<typename Has, typename T>
struct can_only_construct<Has,T>{
    static constexpr bool check_value = !std::is_same_v<Has,T> && std::is_constructible_v<Has,T>;
    using type = std::conditional_t<check_value, std::true_type, std::false_type>;
    static constexpr bool value = type::value;
};

template<typename Has, typename T>
struct template_can_only_construct{
    template<template<typename...> typename TT, typename...Ts>
    static constexpr can_only_construct<Has, Ts...> get_list(TT<Ts...>*);
    using type = decltype(get_list(std::declval<T*>()));
    static constexpr bool value = type::value;
};

template<typename Has, typename T>
constexpr bool template_can_only_construct_v = template_can_only_construct<Has,T>::value;

template<size_t N, typename T, typename...Ts>
struct conv {
    using Next_t = conv<N - 1, T, Ts...>;

    T operator()(const std::variant<Ts...> &var) {
        Next_t next;
        using Cur_t = at_index_t<N,Ts...>;
        if constexpr(std::is_constructible_v<T, Cur_t>) {
            if (std::holds_alternative<Cur_t>(var)) {
                return T(std::get<Cur_t>(var));
            } else {
                return next(var);
            }
        } else {
            return next(var);
        }
    }
};

template<typename T, typename...Ts>
struct conv<0, T, Ts...> {
    T operator()(const std::variant<Ts...> &var) {
        using Cur_t = at_index_t<0,Ts...>;
        if constexpr(std::is_constructible_v<T, Cur_t>) {
            if (std::holds_alternative<Cur_t>(var)) {
                return T(std::get<Cur_t>(var));
            }
        }
        if constexpr(std::is_default_constructible_v<T>) {
            return T{};
        }
        throw std::bad_variant_access();
    }
};

}

template<typename...Ts>
class extended_variant {
        using Data_t = std::variant<Ts...>;
    private:
        Data_t data;

    public:
        template<typename=std::enable_if_t<std::is_default_constructible_v<Data_t>>>
        extended_variant() : data{} {};

        extended_variant(const Data_t& val) : data{val} {};

        extended_variant(const extended_variant &other) : data{other.data}{};

        extended_variant(extended_variant &&other) noexcept : data{std::move(other.data)}{};

        template<typename T, typename=std::enable_if_t<std::is_constructible_v<Data_t,T>>>
        extended_variant(T &&val) : data{std::forward<T>(val)} {};

        extended_variant &operator=(const extended_variant &other) = default;

        extended_variant &operator=(extended_variant &&other) noexcept = default;

        extended_variant &operator=(const Data_t &val) {
            data = val;
            return *this;
        }

        template<typename T, typename=std::enable_if_t<std::is_constructible_v<Data_t,T>>>
        extended_variant &operator=(T &&val) {
            data = std::forward<T>(val);
            return *this;
        }

        template<typename T, typename=std::enable_if_t<impl::template_has_or_can_construct_v<T, Data_t>>,
                typename CT=impl::conv<sizeof...(Ts)-1, T, Ts...>>
        operator T() const {
            CT converter;
            if constexpr(impl::template_has_v<T,Data_t>){
                if (std::holds_alternative<T>(data)){
                    return std::get<T>(data);
                }
            }
            return converter(data);
        }

        template<typename T>
        bool holds() const {
            if constexpr(!impl::template_has_v<T, Data_t>){
                return false;
            } else return std::holds_alternative<T>(data);
        }

        const Data_t& get() const {
            return data;
        }

        Data_t& get() {
            return data;
        }

        template<typename T, typename=std::enable_if_t<impl::template_has_v<T,Data_t>>>
        T get() const {
            return std::get<T>(data);
        }

        template<size_t N, typename=std::enable_if_t< 0<N && N<sizeof...(Ts)-1 >>
        decltype(auto) get() const {
            return std::get<N>(data);
        };

        friend std::ostream &operator<<(std::ostream &os, const extended_variant& self){
            std::visit([&](auto &&arg){ os << arg; }, self.data);
            return os;
        }

        friend std::istream &operator>>(std::istream &is, extended_variant& self){
            std::visit([&](auto &arg){ is >> arg;}, self.data);
            return is;
        }

};
