#pragma once

// from https://osyo-manga.hatenadiary.org/entry/20111001/1317460627

#include <cstddef>
#include <limits>

namespace ce{

template<typename Char, std::size_t N>
struct c_string{
    Char elems[N ? N : 1];
    constexpr Char const* c_str() const{
        return &elems[0];
    }
};

template<std::size_t... Indices>
struct index_tuple{};

template<
    std::size_t Start,
    std::size_t Finish,
    std::size_t Step = 1,
    typename Acc = index_tuple<>,
    bool Break = (Start >= Finish)
>
struct index_range{
    typedef Acc type;
};

template<
    std::size_t Start, std::size_t Finish, std::size_t Step,
    std::size_t... Indices
>
struct index_range<Start, Finish, Step, index_tuple<Indices...>, false>
    : index_range<Start + Step, Finish, Step, index_tuple<Indices..., Start>>
{};

constexpr char
itoc(int n){
    return 0 <= n && n <= 9 ? '0' + n
        : '\0';
}

template<typename Char, std::size_t N, std::size_t ...Indices>
constexpr c_string<Char, N>
itoa_push_front_impl(Char const (&str)[N], index_tuple<Indices...>, Char c){
    return {{ c, str[Indices]..., '\0' }};
}

template<typename Char, std::size_t N>
constexpr c_string<Char, N>
itoa_push_front(Char const (&str)[N], Char c){
    return itoa_push_front_impl(str, typename index_range<0, N-2>::type_(), c);
}

template<typename Char, std::size_t N>
constexpr c_string<Char, N>
itoa_push_front(c_string<Char, N> const& str, Char c){
    return itoa_push_front(str.elems, c);
}

template<typename T, typename String>
constexpr String
itoa_impl(T n, String const& str){
    return n == 0 ? str
        : itoa_impl(n/10, itoa_push_front(str, itoc(n%10)));
}

template<typename T, typename String>
constexpr auto
itoa(T n, String const& str)->decltype(itoa_impl(n, str)){
    return n < 0 ? itoa_push_front(itoa_impl(n*-1, str), '-')
         : itoa_impl(n, str);
}

template<typename T>
constexpr c_string<char, std::numeric_limits<T>::digits10+3>
itoa(T n){
    return itoa(n, c_string<char, std::numeric_limits<T>::digits10+3>{{}});
}


template<typename Char>
constexpr bool
equal(Char const* a, Char const* b){
    return a[0] == Char('\0') && b[0] == Char('\0') ? true
         : a[0] == b[0]                             ? equal(a+1, b+1)
         : false;
}

}  // namespace ce