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

template<typename Lam, int=(Lam{}(),0)>
constexpr bool is_constexpr(Lam) {
    return true;
}

constexpr bool is_constexpr(...){
    return false;
}

template<size_t N, typename T, typename...Ts>
struct conv {
    using Next_t = conv<N - 1, T, Ts...>;

    constexpr conv() {};

    constexpr T operator()(const std::variant<Ts...> &var) const {
        constexpr Next_t next;
        using Cur_t = at_index_t<N,Ts...>;
        if constexpr(std::is_constructible_v<T, Cur_t>) {
            if (std::holds_alternative<Cur_t>(var)) {
                return T(std::get<Cur_t>(var));
            }
        }
        return next(var);
    }
};

template<typename T, typename...Ts>
struct conv<0, T, Ts...> {
    constexpr conv() {};

    constexpr std::remove_const_t<T> operator()(const std::variant<Ts...> &var) const {
        using Cur_t = at_index_t<0,Ts...>;
        if constexpr(std::is_constructible_v<T, Cur_t>) {
            if (std::holds_alternative<Cur_t>(var)) {
                return T(std::get<Cur_t>(var));
            }
        }
        if constexpr(std::is_default_constructible_v<T>) {
            return T{};
        } else static_assert(std::is_default_constructible_v<T>, "convertable must have default constructor");
    }

};

}

template<typename...Ts>
class extended_variant {
        using Data_t = std::variant<Ts...>;
    private:
        Data_t data;

    public:
        constexpr static int npos = -1;

        template<typename=std::enable_if_t<std::is_default_constructible_v<Data_t>>>
        constexpr extended_variant() : data{} {};

        constexpr extended_variant(const Data_t& val) : data{val} {};

        constexpr extended_variant(const extended_variant &other) : data{other.data}{};

        constexpr extended_variant(extended_variant &&other) noexcept : data{std::move(other.data)}{};

        template<typename T, typename=std::enable_if_t<std::is_constructible_v<Data_t,T>>>
        constexpr extended_variant(T &&val) : data{std::forward<T>(val)} {};

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
        constexpr operator T() const {
            constexpr CT converter;
            if constexpr(impl::template_has_v<T,Data_t>){
                if (std::holds_alternative<T>(data)){
                    return std::get<T>(data);
                }
            }
            return converter(data);
        }

        template<typename T>
        constexpr bool holds() const {
            if constexpr(!impl::template_has_v<T, Data_t>){
                return false;
            } else return std::holds_alternative<T>(data);
        }

        template<typename F, typename RT=decltype(std::visit(std::declval<F>(),data))>
        constexpr RT visit(F &&functor) const {
            return std::visit(std::forward<F>(functor), data);
        }

        constexpr size_t index() const noexcept {
            return data.index();
        }

        void swap(extended_variant& other) noexcept {
            data.swap(other.data);
        }

        constexpr Data_t get_variant() const {
            return data;
        }

        extended_variant set_variant(Data_t var){
            data = std::move(var);
            return *this;
        }

        template<typename T, typename=std::enable_if_t<impl::template_has_v<T,Data_t>>>
        constexpr decltype(auto) get_if() const {
            return std::get_if<T>(&data);
        }

        template<size_t N, typename=std::enable_if_t< 0<N && N<sizeof...(Ts)-1 >>
        constexpr decltype(auto) get_if() const {
            return std::get_if<N>(&data);
        };

        template<typename T, typename=std::enable_if_t<impl::template_has_v<T,Data_t>>>
        constexpr decltype(auto) get() const {
            return std::get<T>(data);
        }

        template<size_t N, typename=std::enable_if_t< 0<N && N<sizeof...(Ts)-1 >>
        constexpr decltype(auto) get() const {
            return std::get<N>(data);
        };

        constexpr bool operator==(const extended_variant& other) const {
            return data == other.data;
        }

        constexpr bool operator!=(const extended_variant& other) const {
            return data != other.data;
        }

        constexpr bool operator<(const extended_variant& other) const {
            return data < other.data;
        }

        constexpr bool operator>(const extended_variant& other) const {
            return data > other.data;
        }

        constexpr bool operator<=(const extended_variant& other) const {
            return data <= other.data;
        }

        constexpr bool operator>=(const extended_variant& other) const {
            return data >= other.data;
        }

        friend std::ostream &operator<<(std::ostream &os, const extended_variant& self){
            std::visit([&](auto &&arg){ os << arg; }, self.data);
            return os;
        }

        friend std::istream &operator>>(std::istream &is, extended_variant& self){
            std::visit([&](auto &arg){ is >> arg;}, self.data);
            return is;
        }

};

namespace std{

template<typename...Ts>
struct hash<extended_variant<Ts...>> {
    size_t operator()(const extended_variant<Ts...>& ev){
        return ev.get_variant();
    }
};

template<typename...Ts>
void swap(extended_variant<Ts...>& lhs, extended_variant<Ts...>& rhs) noexcept {
    lhs.swap(rhs);
}

template<typename Visitor, typename...Ts>
constexpr decltype(auto) visit(Visitor&& vis, extended_variant<Ts...> var){
    return std::visit(std::forward<Visitor>(vis), var.get_variant());
}

template<size_t N, typename...Ts>
constexpr decltype(auto) get(const extended_variant<Ts...>& ev) noexcept {
    return ev.template get<N>();
}

template<typename T, typename...Ts>
constexpr decltype(auto) get(const extended_variant<Ts...>& ev) noexcept {
    return ev.template get<T>();
}

template<typename T, typename...Ts>
constexpr T* get_if(const extended_variant<Ts...>& ev) noexcept {
    return ev.template get_if<T>();
}

template<size_t N, typename...Ts>
constexpr decltype(auto) get_if(const extended_variant<Ts...>& ev) noexcept {
    return ev.template get_if<N>();
}

template<typename T, typename...Ts>
constexpr bool holds_alternative(const extended_variant<Ts...>& ev) noexcept {
    return ev.template holds<T>();
}

template<typename T, typename A>
class vector;

template<typename T, size_t N>
class array;

}

template<typename...Ts>
using hvector = std::vector<extended_variant<Ts...>>;

template<size_t N, typename...Ts>
using harray = std::array<extended_variant<Ts...>, N>;

template<typename...Ts>
using evariant = extended_variant<Ts...>;