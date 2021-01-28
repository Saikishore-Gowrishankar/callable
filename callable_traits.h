#pragma once
//Type traits for internal comparison
//is_equality_comparable: checks to see if equality comparisons work

//Copyright (c) Saikishore Gowrishankar 2020. All rights reserved.
//Licence: MIT

namespace
{
    template<typename T>
    struct is_equality_comparable
    {
    private:
        static void* boolean_test(bool);
        template<typename U>
        static std::true_type comp(decltype(boolean_test(std::declval<U const&>() == std::declval<U const&>()) ),
                                   decltype(!(boolean_test(std::declval<U const&>() == std::declval<U const&>()))));
        //Basis case
        template<typename U>
        static std::false_type comp(...);
    public:
        static constexpr bool value = decltype(comp<T>(nullptr, nullptr))::value;
    };

    //True case: operator== is defined and boolean conversions succeeded
    template<typename T, bool test = is_equality_comparable<T>::value>
    struct equal_comp
    {
        static bool equals(T const& a, T const& b)  { return a == b; }
    };

    //Basis case, throws std::logic_error
    template<typename T>
    struct equal_comp<T, false>
    {
         [[noreturn]] static bool equals([[maybe_unused]] T const&,
                                         [[maybe_unused]] T const&)
         {
             throw std::logic_error("Not equality comparable");
         }
    };
}
