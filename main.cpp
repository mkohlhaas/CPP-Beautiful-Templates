#include <array>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <vector>

// Kind of Template | Type deduction | Full specialization allowed ? | Partial specialization allowed ? |
// -----------------+----------------+-------------------------------+-----------------------------------
//     Function     |     Yes        |            Yes                |             No                   |
//     Class        |     Yes*       |            Yes                |             Yes                  |
//     Variable     |     Yes*       |            Yes                |             Yes                  |
//     Alias        |     No         |            No                 |             No                   |
// -----------------+----------------+-------------------------------+-----------------------------------

// Variable templates are syntactic sugar for class templates.
// A variable template is exactly 100% equivalent to a static data member of a class template.

// * C++17 has CTAD = Class Template Argument Deduction

namespace deducing_Tref_not_Trefref
{
    template <typename T>
    void
    f(T &)
    {
        puts(__PRETTY_FUNCTION__);
    }

} // namespace deducing_Tref_not_Trefref

namespace rvalues_are_kinda_like_const_lvalues
{
    template <typename T>
    void
    f(T &)
    {
        puts(__PRETTY_FUNCTION__);
    }

} // namespace rvalues_are_kinda_like_const_lvalues

namespace how_to_partially_specialize_a_function
{
    // How to partially specialize a function

    // primary template
    template <typename T>
    struct is_pointer_impl
    {
        static bool
        _()
        {
            return false;
        }
    };

    // partial specialization (classes can be partially specialized!)
    template <typename T>
    struct is_pointer_impl<T *>
    {
        static bool
        _()
        {
            return true;
        }
    };

    template <typename T>
    bool
    is_pointer(T)
    {
        return is_pointer_impl<T>::_();
    }

} // namespace how_to_partially_specialize_a_function

namespace good_tag_dispatch
{
    // tree

    template <typename Element>
    struct tree_iterator
    {
        tree_iterator &operator++();
        using supports_plus = std::false_type;
    };

    template <typename Element>
    struct tree
    {
        using iterator = tree_iterator<Element>;
    };

    // vector

    template <typename Element>
    struct vector_iterator
    {
        vector_iterator &operator++();
        vector_iterator  operator+();
        using supports_plus = std::true_type;
    };

    template <typename Element>
    struct vector
    {
        using iterator = vector_iterator<Element>;
    };

    // advance algo

    template <typename Iter>
    Iter
    advance_impl(Iter begin, int n, std::false_type)
    {
        for (int i = 0; i < n; i++)
        {
            ++begin;
        }
        return begin;
    }

    template <typename Iter>
    Iter
    advance_impl(Iter begin, int n, std::true_type)
    {
        return begin + n;
    }

    template <typename Iter>
    auto
    advance(Iter begin, int n)
    {
        return advance_impl(begin, n, Iter::supports_plus());
    }

} // namespace good_tag_dispatch

namespace template_auto
{
    template <auto... x>
    struct foo
    {
        void
        f()
        {
            puts(__PRETTY_FUNCTION__);
        }
    };
} // namespace template_auto

namespace variadic_function_templates
{
    // Variadic function templates

    // Overloaded functions. Template functions cannot be partially specialized!

    template <typename T>
    T
    min(T a)
    {
        return a;
    }

    template <typename T>
    T
    min(T a, T b)
    {
        return a < b ? a : b;
    }

    template <typename T, typename... Ts> // T1, T2, T3, ...
    T
    min(T a, Ts... as)                    // T1 a1, T2 a2, T3, a3, ...
    {
        return min(a, min(as...));        // a1, a2, a3, ...
    }
} // namespace variadic_function_templates

namespace type_sizes
{
    template <typename... Ts>
    auto
    get_type_sizes()
    {
        // sizeof...(Ts) = number of elements in Ts
        // sizeof(Ts)... = sizeof T1, sizeof T2, sizeof T3, ...
        return std::array<std::size_t, sizeof...(Ts)>{sizeof(Ts)...};

        // return std::array{sizeof(Ts)...}; // same
    }
} // namespace type_sizes

namespace summation
{
    template <typename... Ts>
    std::common_type_t<Ts...>
    sum(Ts... args)
    {
        return (... + args); // unary left fold
    }
} // namespace summation

namespace tuple_template
{
    // primary
    template <typename T, typename... Ts>
    struct tuple
    {
        tuple(T const &t, Ts const &...ts) : value(t), rest(ts...)
        {
        }

        constexpr int
        size() const
        {
            return 1 + rest.size();
        }

        T            value;
        tuple<Ts...> rest;
    };

    // partial specialization
    template <typename T>
    struct tuple<T>
    {
        tuple(const T &t) : value(t)
        {
        }

        constexpr int
        size() const
        {
            return 1;
        }

        T value;
    };

    // primary
    template <size_t N, typename T, typename... Ts>
    struct nth_type : nth_type<N - 1, Ts...> // recursive inheritance
    {
        static_assert(N < sizeof...(Ts) + 1, "index out of bounds");
    };

    // partial specialization
    template <typename T, typename... Ts>
    struct nth_type<0, T, Ts...>
    {
        using value_type = T;
    };

    template <size_t N, typename... Ts>
    using nth_type_t = nth_type<N, Ts...>::value_type;

    // primary
    template <size_t N>
    struct getter
    {
        template <typename... Ts>
        static nth_type_t<N, Ts...> &
        get(tuple<Ts...> &t)
        {
            return getter<N - 1>::get(t.rest);
        }
    };

    // explicit specialization
    template <>
    struct getter<0>
    {
        template <typename T, typename... Ts>
        static T &
        get(tuple<T, Ts...> &t)
        {
            return t.value;
        }
    };

    // API
    template <size_t N, typename... Ts>
    nth_type_t<N, Ts...> &
    get(tuple<Ts...> &t)
    {
        return getter<N>::get(t);
    }
} // namespace tuple_template

namespace fold_expressions
{
    template <typename... T>
    void
    print1(T... args)
    {
        (..., (std::cout << args)) << '\n'; // comma operator
    }

    template <typename... T>
    void
    print2(T... args)
    {
        (std::cout << ... << args) << '\n'; // binary left fold (init expression = std::cout)
    }

    template <typename T, typename... Args>
    void
    push_back_many(std::vector<T> &v, Args &&...args)
    {
        (..., v.push_back(args));
    }
} // namespace fold_expressions

namespace factorial_class_template
{
    // class template (with static is )

    template <unsigned int N>
    struct factorial
    {
        static constexpr unsigned int value = N * factorial<N - 1>::value;
    };

    template <>
    struct factorial<0>
    {
        static constexpr unsigned int value = 1;
    };

    // API
    template <unsigned int N>
    constexpr unsigned int factorial_v = factorial<N>::value;
} // namespace factorial_class_template

namespace factorial_variable_template
{
    // variable template (just syntactic sugar for class template with static member)

    template <unsigned int N>
    constexpr unsigned int factorial = N * factorial<N - 1>;

    template <>
    constexpr unsigned int factorial<0> = 1;
} // namespace factorial_variable_template

namespace factorial_function_template
{
    // function template

    template <unsigned int n>
    constexpr unsigned int
    factorial()
    {
        return n * factorial<n - 1>();
    }

    template <>
    constexpr unsigned int
    factorial<0>()
    {
        return 1;
    }
} // namespace factorial_function_template

namespace factorial_constexpr
{
    constexpr unsigned int
    factorial(unsigned int const n)
    {
        return n > 1 ? n * factorial(n - 1) : 1;
    }
} // namespace factorial_constexpr

int
main()
{
    std::cout << std::boolalpha;

    {
        // Forwarding references (T&&) are too easy. Everything works with them.

        using namespace deducing_Tref_not_Trefref;

        std::cout << "\n=== Deducing T&, not T&&\n" << std::endl;

        int i = 42;

        // pass l-value ref
        f(static_cast<int &>(i)); // [T=int]

        // pass volatile l-value ref
        f(static_cast<volatile int &>(i)); // [T=volatile int]

        // pass const l-value ref
        f(static_cast<const int &>(i)); // [T=const int]

        // pass const r-value ref
        f(static_cast<const int &&>(i)); // NOTE: [T=const int] (!)

        // pass r-value ref
        // f(static_cast<int &&>(i));          // ERROR

        // pass volatile r-value ref
        // f(static_cast<volatile int &&>(i)); // ERROR
    }

    {
        using namespace rvalues_are_kinda_like_const_lvalues;

        std::cout << "\n=== r-Values are kinda like const lvalues\n" << std::endl;

        int i = 42;

        // pass l-value ref
        f(static_cast<int &>(i)); // [T = int]

        // pass const l-value ref
        f(static_cast<const int &>(i)); // [T = const int]

        // pass const r-value ref (acts like const l-value ref)
        f(static_cast<const int &&>(i)); // [T = const int]

        // pass r-value ref
        // f(static_cast<int &&>(i));    // error
    }

    {
        using namespace how_to_partially_specialize_a_function;

        std::cout << "\n=== Function Templates can't be partially specialized - only fully!\n" << std::endl;

        int i{};
        std::cout << is_pointer(i) << std::endl;  // false
        std::cout << is_pointer(&i) << std::endl; // true
    }

    {
        using namespace template_auto;

        std::cout << "\n=== auto Templates ===\n" << std::endl;

        // each parameter is deduced independently
        foo<42, 42.0, false, 'x'> myFoo;
        myFoo.f(); // [with auto ...x = {42, 4.2e+1, false, 'x'}]
    }

    {
        std::cout << "\n=== Lambda Templates ===\n" << std::endl;

        auto l1 = [](int a) { return a + a; };           // regular lambda (C++11)
        auto l2 = [](auto a) { return a + a; };          // generic lambda (C++14)
        auto l3 = []<typename T>(T a) { return a + a; }; // template lambda (C++20)

        int v1 = l1(21.0);
        int v2 = l2(21.0);
        int v3 = l3(21.0);

        std::cout << v1 << std::endl; // 42
        std::cout << v2 << std::endl; // 42
        std::cout << v3 << std::endl; // 42
    }

    {
        using namespace variadic_function_templates;

        std::cout << "\n=== Variadic Function Templates ===\n" << std::endl;

        std::cout << min(7.5) << std::endl;            // 7.5
        std::cout << min(42.0, 7.5) << std::endl;      // 7.5
        std::cout << min(1, 5, 3, -4, 9) << std::endl; // -4
    }

    {
        using namespace type_sizes;

        std::cout << "\n=== sizeof() & Templates ===\n" << std::endl;

        auto sizes = get_type_sizes<short, int, long, long long>();

        for (auto const s : sizes)
        {
            std::cout << s << std::endl; // 2 4 8 8
        }
    }

    {
        using namespace summation;

        std::cout << "\n=== Fold Expressions ===\n" << std::endl;

        // Four types of folds: (op is a binary operator)
        // unary  right fold (E op ...)      -> (E1 op (... op (EN-1 op EN)))
        // unary  left  fold (... op E)      -> (((E1 op E2) op ...) op EN)
        // binary right fold (E op ... op I) -> (E1 op (... op (EN−1 op (EN op I))))
        // binary left  fold (I op ... op E) -> ((((I op E1) op E2) op ...) op EN)

        int n = sum(1, 2, 3, 4, 5);

        std::cout << n << std::endl; // 15
    }

    {
        using namespace tuple_template;

        std::cout << "\n=== Tuples ===\n" << std::endl;

        tuple one(42);
        tuple two(42, 42.5);
        tuple three(42, 42.5, 'a');

        std::cout << get<0>(one) << std::endl;                                                   // 42
        std::cout << get<0>(two) << " " << get<1>(two) << std::endl;                             // 42 42.5
        std::cout << get<0>(three) << " " << get<1>(three) << " " << get<2>(three) << std::endl; // 42 42.5 a
    }

    {
        using namespace fold_expressions;

        std::cout << "\n=== Fold Expressions ===\n" << std::endl;

        print1('d', 'o', 'g'); // dog
        print2('d', 'o', 'g'); // dog

        std::vector<int> v;
        push_back_many(v, 1, 2, 3, 4, 5);

        std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " ")); // 1 2 3 4 5
    }

    {
        using namespace factorial_class_template;

        std::cout << "\n\n=== Factorial Class Template ===\n" << std::endl;

        std::cout << factorial_v<0> << std::endl;  // 1
        std::cout << factorial_v<1> << std::endl;  // 1
        std::cout << factorial_v<2> << std::endl;  // 2
        std::cout << factorial_v<3> << std::endl;  // 6
        std::cout << factorial_v<4> << std::endl;  // 24
        std::cout << factorial_v<5> << std::endl;  // 120
        std::cout << factorial_v<12> << std::endl; // 479001600
    }

    {
        using namespace factorial_variable_template;

        std::cout << "\n=== Factorial Variable Template ===\n" << std::endl;

        std::cout << factorial<0> << std::endl;  // 1
        std::cout << factorial<1> << std::endl;  // 1
        std::cout << factorial<2> << std::endl;  // 2
        std::cout << factorial<3> << std::endl;  // 6
        std::cout << factorial<4> << std::endl;  // 24
        std::cout << factorial<5> << std::endl;  // 120
        std::cout << factorial<12> << std::endl; // 479001600
    }

    {
        using namespace factorial_function_template;

        std::cout << "\n=== Factorial Function Template ===\n" << std::endl;

        std::cout << factorial<0>() << std::endl;  // 1
        std::cout << factorial<1>() << std::endl;  // 1
        std::cout << factorial<2>() << std::endl;  // 2
        std::cout << factorial<3>() << std::endl;  // 6
        std::cout << factorial<4>() << std::endl;  // 24
        std::cout << factorial<5>() << std::endl;  // 120
        std::cout << factorial<12>() << std::endl; // 479001600
    }

    {
        using namespace factorial_constexpr;

        std::cout << "\n=== Factorial Constexpr ===\n" << std::endl;

        std::cout << factorial(0) << std::endl;  // 1
        std::cout << factorial(1) << std::endl;  // 1
        std::cout << factorial(2) << std::endl;  // 2
        std::cout << factorial(3) << std::endl;  // 6
        std::cout << factorial(4) << std::endl;  // 24
        std::cout << factorial(5) << std::endl;  // 120
        std::cout << factorial(12) << std::endl; // 479001600
    }
}
