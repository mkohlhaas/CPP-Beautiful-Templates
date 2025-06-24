#include <array>
#include <cxxabi.h>
#include <iostream>
#include <iterator>
#include <ostream>
#include <print>
#include <string>
#include <type_traits>
#include <vector>

using namespace std::string_literals; // enables s-suffix for std::string literals

// #pragma GCC diagnostic ignored "-Wunused"

std::string
demangle(const char *mangled_name)
{
    std::string result;
    std::size_t len    = 0;
    int         status = 0;
    char       *ptr    = __cxxabiv1::__cxa_demangle(mangled_name, nullptr, &len, &status);

    if (status == 0)
    {
        result = ptr;
    }
    else
    {
        result = "demangle error";
    }

    ::free(ptr);

    return result;
}

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

namespace enable_if_template
{
    // widget (uses write())

    struct widget
    {
        int         id;
        std::string name;

        std::ostream &
        write(std::ostream &os) const
        {
            os << id << ',' << name << '\n';
            return os;
        }
    };

    // gadget (uses ostream)

    struct gadget
    {
        int         id;
        std::string name;

        friend std::ostream &operator<<(std::ostream &, gadget const &);
    };

    std::ostream &
    operator<<(std::ostream &os, gadget const &g)
    {
        os << g.id << ',' << g.name << '\n';
        return os;
    }

    // uses_write
    // TODO: Should be part of widget, gadget (but this is about enable_if so we don't care).
    // TODO: Class template with static member is a variable template.

    template <typename T>
    struct uses_write
    {
        static constexpr bool value = false;
    };

    template <>
    struct uses_write<widget>
    {
        static constexpr bool value = true;
    };

    template <typename T>
    inline constexpr bool uses_write_v = uses_write<T>::value;

    // enable_if

    template <bool B, typename T = void>
    struct enable_if
    {
    };

    // only for true we have a `type`
    template <typename T>
    struct enable_if<true, T>
    {
        using type = T;
    };

    template <bool B, typename T = void>
    using enable_if_t = enable_if<B, T>::type;

    // serialize

    template <typename T, enable_if_t<uses_write_v<T>> * = nullptr>
    void
    serialize(std::ostream &os, T const &value)
    {
        value.write(os);
    }

    template <typename T, enable_if_t<not uses_write_v<T>> * = nullptr>
    void
    serialize(std::ostream &os, T const &value)
    {
        os << value;
    }
} // namespace enable_if_template

namespace sfinae_error
{
    // char (*)[N % 2 == 0] either creates char (*)[true] = char(*)[1] or char (*)[false] = char(*)[0].
    // The latter - array of size 0 - is a SFINAE error.
    // See https://en.cppreference.com/w/cpp/language/sfinae (lists type errors that are SFINAE errors)

    template <typename T, size_t N>
    void
    handle(T (&)[N], char (*)[N % 2 == 0] = nullptr) // accepts array with even number of elements
    {
        std::println("handle even array: {} elements", N);
    }

    template <typename T, size_t N>
    void
    handle(T (&)[N], char (*)[N % 2 == 1] = nullptr) // accepts array with odd number of elements
    {
        std::println("handle odd  array: {} elements", N);
    }
} // namespace sfinae_error

namespace decltype_templates
{
    namespace
    {
        template <typename T>
        struct foo
        {
            using foo_type = T;
        };

        template <typename T>
        struct bar
        {
            using bar_type = T;
        };

        template <typename T>
        struct dummy
        {
            using dummy_type = T;
        };
    } // namespace

    namespace
    {
        template <typename T>
        decltype(typename T::foo_type(), void()) // NOTE: comma operator
        handle(T const &)
        {
            std::println("handle a foo");
        }

        template <typename T>
        decltype(typename T::bar_type(), void()) // NOTE: comma operator
        handle(T const &)
        {
            std::println("handle a bar");
        }
    } // namespace
} // namespace decltype_templates

namespace common_type
{
    namespace
    {
        template <typename, typename... Ts>
        struct has_common_type : std::false_type
        {
        };

        template <typename... Ts>
        struct has_common_type<std::void_t<std::common_type_t<Ts...>>, Ts...> : std::true_type
        {
        };

        template <typename... Ts>
        constexpr bool has_common_type_v = sizeof...(Ts) < 2 || has_common_type<void, Ts...>::value;
    } // namespace

    template <typename... Ts, typename = std::enable_if_t<has_common_type_v<Ts...>>>
    void
    process(Ts &&...)
    {
        std::println("{}", demangle(typeid(std::common_type_t<Ts...>).name()));
    }
} // namespace common_type

namespace constraints_concepts
{
    // A CONSTRAINT is a modern way to define requirements on template parameters.
    // A CONSTRAINT is a predicate that evaluates to true or false at compile-time.
    // A CONCEPT is a set of named constraints

    namespace no_check
    {
        template <typename T>
        T
        add(T const a, T const b)
        {
            return a + b;
        }
    } // namespace no_check

    namespace enable_if_check
    {
        template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        T
        add(T const a, T const b)
        {
            return a + b;
        }
    } // namespace enable_if_check

    namespace static_assert_check
    {
        template <typename T>
        T
        add(T const a, T const b)
        {
            static_assert(std::is_arithmetic_v<T>, "Arithmetic type required");
            return a + b;
        }
    } // namespace static_assert_check

    namespace requires_check
    {
        // REQUIRES CLAUSE doesn't use curly braces.

        template <typename T>
            requires std::is_arithmetic_v<T>
        T
        add(T const a, T const b)
        {
            return a + b;
        }
    } // namespace requires_check

    namespace requires_check_alternative
    {
        // the same, only different order (matter of personal taste)

        template <typename T>
        T
        add(T const a, T const b)
            requires std::is_arithmetic_v<T>
        {
            return a + b;
        }
    } // namespace requires_check_alternative

    namespace concept_check
    {
        // REQUIRES EXPRESSION uses curly braces.
        // A requires expression is a Boolean expression that can be used with a requires clause.

        template <typename T>
        concept arithmetic = requires { std::is_arithmetic_v<T>; };

        template <arithmetic T> // used like a data type
        T
        add(T const a, T const b)
        {
            return a + b;
        }
    } // namespace concept_check

    namespace concept_check_alternative
    {
        // simple requirement (doesn't use `requires` keyword)
        template <typename T>
        concept arithmetic = std::is_arithmetic_v<T>;

        template <arithmetic T>
        T
        add(T const a, T const b)
        {
            return a + b;
        }
    } // namespace concept_check_alternative

    namespace old_requirements_style
    {
        // old style
        template <typename T, typename U = void>
        struct is_container : std::false_type
        {
        };

        template <typename T>
        struct is_container<T, std::void_t<typename T::value_type,               //
                                           typename T::size_type,                //
                                           typename T::allocator_type,           //
                                           typename T::iterator,                 //
                                           typename T::const_iterator,           //
                                           decltype(std::declval<T>().size()),   //
                                           decltype(std::declval<T>().begin()),  //
                                           decltype(std::declval<T>().end()),    //
                                           decltype(std::declval<T>().cbegin()), //
                                           decltype(std::declval<T>().cend())>>  //
            : std::true_type
        {
        };

        template <typename T, typename U = void>
        constexpr bool is_container_v = is_container<T, U>::value;
    } // namespace old_requirements_style

    namespace new_requirements_style
    {
        // NOTE: naming convention (`container` instead of `is_container` bc it's used like a data type)
        template <typename T>
        concept container = requires(T t) {
            typename T::value_type;     // type requirement
            typename T::size_type;      // ..
            typename T::allocator_type; // ..
            typename T::iterator;       // ..
            typename T::const_iterator; // type requirement
            t.size();                   // simple requirement
            t.begin();                  // ..
            t.end();                    // ..
            t.cbegin();                 // ..
            t.cend();                   // simple requirement
        };

        template <container C>
        void
        process(C &&)
        {
            // ...
        }
    } // namespace new_requirements_style
} // namespace constraints_concepts

namespace simple_requirements
{
    // syntax: requires (parameter-list) { requirement-seq }

    template <typename T>
    concept arithmetic = requires { std::is_arithmetic_v<T>; };

    template <typename T>
    concept arithmetic_alt = std::is_arithmetic_v<T>; // empty parameter list can be omitted

    template <typename T>
    concept addable = requires(T a, T b) { a + b; };

    template <typename T>
    concept logger = requires(T t) {
        t.error("just"); // takes a single parameter of `const char*` or some type that can be constructed from it
        t.warning("a");  // the actual values passed as arguments have no importance (they are never performed)
        t.info("demo");  // they are only checked for type correctness
    };

    template <logger Logger>
    void
    log_error(Logger &l)
    {
        l.error("error");
        l.warning("warning");
        l.info("info");
    }

    struct console_logger
    {
        void
        error(std::string_view msg)
        {
            std::cout << msg << std::endl;
        }

        void
        warning(std::string_view msg)
        {
            std::cout << msg << std::endl;
        }

        void
        info(std::string_view msg)
        {
            std::cout << msg << std::endl;
        }
    };
} // namespace simple_requirements

namespace compound_requirement
{
    namespace
    {
        template <typename T>
        void
        f(T) noexcept // doesn't throw errors
        {
            // ...
        }

        template <typename T>
        void
        g(T) // could throw errors
        {
            // ...
        }
    } // namespace

    namespace
    {
        // We want to check an expression for exceptions and/or return types.
        // Syntax: { expression } [noexcept] [-> type_constraint];
        // (both `noexcept` and `type_constraint` are optional)

        template <typename F, typename... Args>
        concept NonThrowing = requires(F &&func, Args... args) {
            { func(args...) } noexcept; // checking for noexcept specifier
        };

        template <typename F, typename... Args>
            requires NonThrowing<F, Args...>
        void
        invoke(F &&func, Args... args)
        {
            func(args...);
        }
    } // namespace
} // namespace compound_requirement

namespace check_for_return_types
{
    // checking for return values (must be a type constraint; not the actual return type)

    template <typename T>
    concept timer = requires(T t) {
        { t.start() } -> std::same_as<void>;            // start() returns void
        { t.stop() } -> std::convertible_to<long long>; // stop() returns anything convertible to long long
    };

    struct timerA
    {
        void
        start()
        {
            // ...
        }

        int
        // long long
        stop()
        {
            return 0;
        }
    };
} // namespace check_for_return_types

namespace nested_requirements
{
    // std::conjunction_v performs a logical AND on the sequence of traits.
    template <typename T, typename... Ts>
    inline constexpr bool are_same_v = std::conjunction_v<std::is_same<T, Ts>...>;

    template <typename... T>
    concept HomogenousRange = requires(T... t) { // `requires`
        (... + t);                               // simple requirement (no `requires`)
        requires are_same_v<T...>;               // nested requirement: `requires` inside `requires`
        requires sizeof...(T) > 1;               // nested requirement: `requires` inside `requires`
    };

    template <typename... T>
        requires HomogenousRange<T...>
    auto
    add(T &&...t)
    {
        return (... + t);
    }
} // namespace nested_requirements

namespace composing_constraints_1
{
    // Composing constraints

    // with && (conjunction) and || (disjunction); short-circuited

    template <typename T>
        requires std::is_integral_v<T> && std::is_signed_v<T>
    T
    decrement(T value)
    {
        return --value;
    }
} // namespace composing_constraints_1

namespace composing_constraints_2
{
    template <typename T>
    concept Integral = std::is_integral_v<T>;

    template <typename T>
    concept Signed = std::is_signed_v<T>;

    template <typename T>
    concept SignedIntegral = Integral<T> && Signed<T>;

    template <SignedIntegral T>
    T
    decrement(T value)
    {
        return --value;
    }
} // namespace composing_constraints_2

namespace constrain_template_parameter_packs
{
    // Conjunctions and disjunctions cannot be used to constrain template parameter packs.

    template <typename... T>
    //  requires std::is_integral_v<T> && ...  // error: this is not allowed
        requires(std::is_integral_v<T> && ...) // now it's a fold expression of type-traits! (But: no short-circuiting!)
    auto
    add(T... args)
    {
        return (args + ...);
    }
} // namespace constrain_template_parameter_packs

namespace concept_template_parameter_packs
{
    // we just give the thing a name (concept = named constraint)
    template <typename T>
    concept Integral = std::is_integral_v<T>; // RHS = boolean value

    // and now it works
    template <typename... T>
        requires(Integral<T> && ...) // fold with concepts creates a conjunction (WITH short-circuiting!)
    auto
    add(T... args)
    {
        return (args + ...);
    }
} // namespace concept_template_parameter_packs

namespace anonymous_concepts_1
{
    template <typename T>
    concept addable = requires(T a, T b) { a + b; }; // requires expression

    template <typename T>
        requires addable<T>                          // requires clause with concept
    auto
    add(T a, T b)
    {
        return a + b;
    }
} // namespace anonymous_concepts_1

namespace anonymous_concepts_2
{
    // confusing syntax (requires requires ...); prefer n637a

    template <typename T>
    //                "anonymous concept"
    //           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        requires requires(T a, T b) { a + b; } // requires clause with requires expression
    auto
    add(T a, T b)
    {
        return a + b;
    }
} // namespace anonymous_concepts_2

namespace abbreviated_function_template
{
    // In C++20 you can use the auto specifier in the function parameter list.
    // This has the effect of transforming the function into a template function.
    // Such a function using auto for function parameters is called an ABBREVIATED FUNCTION TEMPLATE.

    // abbreviated function template
    auto
    add(auto a, auto b)
    {
        return a + b;
    }
} // namespace abbreviated_function_template

namespace constrained_abbreviated_function_template
{
    // constrained abbreviated function template (using concepts)
    auto
    add(std::integral auto a, std::integral auto b)
    {
        return a + b;
    }
} // namespace constrained_abbreviated_function_template

namespace constrained_abbreviated_variadic_function_template
{
    // constrained auto can also be used for variadic function templates

    auto
    add(std::integral auto... args)
    {
        return (args + ...);
    }
} // namespace constrained_abbreviated_variadic_function_template

namespace constrained_auto_with_lambdas
{
    // constrained auto with generic lambdas (C++14)
    auto sum = [](std::integral auto a, std::integral auto b) { return a + b; };

    // constrained auto with templated lambdas (C++20)
    auto twice = []<std::integral T>(T a) { return a + a; };

} // namespace constrained_auto_with_lambdas

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

    {
        using namespace enable_if_template;

        std::cout << "\n=== enable_if ===\n" << std::endl;

        // 'constexpr if' is a compile-time version of the if-statement.
        // The syntax for 'constexpr if' is 'if constexpr(condition)'.

        widget w{1, "one"};

        serialize(std::cout, w); // 1,one
    }

    {
        using namespace sfinae_error;

        std::cout << "\n=== SFINAE ===\n" << std::endl;

        int arr2[]{1, 2, 3, 4};
        handle(arr2); // handle even array: 4 elements

        int arr1[]{1, 2, 3, 4, 5};
        handle(arr1); // handle odd  array: 5 elements
    }

    {
        using namespace decltype_templates;

        std::cout << "\n=== decltype ===\n" << std::endl;

        foo<bool> b_foo;
        bar<bool> b_bar;

        handle(b_foo); // handle a foo
        handle(b_bar); // handle a bar

        dummy<bool> b_dummy [[maybe_unused]];
        // handle(b_dummy); // error: doesn't have a foo_type or bar_type
    }

    {
        using namespace common_type;

        std::cout << "\n=== Common Type ===\n" << std::endl;

        int a = 1;
        process(a);           // int
        process(1);           // int
        process(1, 2, 3);     // int
        process(1, 2.0, '3'); // double

        // process(1, 2.0, "3"); // error
    }

    {
        using namespace constraints_concepts;

        std::cout << "\n=== Parameter Checks ===\n" << std::endl;

        std::cout << no_check::add(2, 4) << std::endl;                   // 6
        std::cout << no_check::add("2"s, "4.0"s) << std::endl;           // 24.0 (Oops!)
        std::cout << enable_if_check::add(2, 4) << std::endl;            // 6
        std::cout << static_assert_check::add(2, 4) << std::endl;        // 6
        std::cout << requires_check::add(2, 4) << std::endl;             // 6
        std::cout << requires_check_alternative::add(2, 4) << std::endl; // 6

        static_assert(old_requirements_style::is_container_v<std::vector<int>>);
        static_assert(new_requirements_style::container<std::vector<int>>);

        new_requirements_style::process(std::vector{1, 2, 3}); // Ok
    }

    {
        using namespace simple_requirements;

        std::cout << "\n=== Simple Requirements ===\n" << std::endl;

        console_logger cl;
        log_error(cl); // error | warning | info
    }

    {
        using namespace compound_requirement;

        std::cout << "\n=== Compound Requirements ===\n" << std::endl;

        invoke(f<int>, 42);

        // invoke(g<int>, 42); // error
    }

    {
        using namespace check_for_return_types;

        std::cout << "\n=== Check Return Types ===\n" << std::endl;

        static_assert(timer<timerA>);
    }

    {
        using namespace nested_requirements;

        std::cout << "\n=== Nested Requirements ===\n" << std::endl;

        static_assert(HomogenousRange<int, int>);
        static_assert(HomogenousRange<int, int, int, int, int, int>);

        static_assert(not HomogenousRange<int>);
        static_assert(not HomogenousRange<int, double>);
        static_assert(not HomogenousRange<float, double>);

        std::println("{}", add(1, 2));     // 3
        std::println("{}", add(1.0, 2.0)); // 3

        // add(1);                         // error
        // add(1, 2.0);                    // error
        // add(1.0f, 2.0);                 // error
    }

    {
        using namespace composing_constraints_1;

        std::cout << "\n=== Composing Constraints 1 ===\n" << std::endl;

        std::println("{}", decrement(5)); // 4

        // std::println("{}", decrement("foo")); // error
    }

    {
        using namespace composing_constraints_2;

        std::cout << "\n=== Composing Constraints 2 ===\n" << std::endl;

        std::println("{}", decrement(5)); // 4

        // std::println("{}", decrement("foo")); // error
    }

    {
        using namespace constrain_template_parameter_packs;

        std::cout << "\n=== Constrain Template Parameter Packs ===\n" << std::endl;

        std::println("{}", add(1, 2, 3));       // 6
        std::println("{}", add(1, 2, 3, 4, 5)); // 15

        // add(1, 42.0); // error
    }
    {
        using namespace concept_template_parameter_packs;

        std::cout << "\n=== Constrain Template Parameter Packs Using Concepts ===\n" << std::endl;

        std::println("{}", add(1, 2, 3));       // 6
        std::println("{}", add(1, 2, 3, 4, 5)); // 15

        // add(1, 42.0); // error
    }

    {
        using namespace anonymous_concepts_1;

        std::cout << "\n=== Anonymous Concepts 1 ===\n" << std::endl;

        std::println("{}", add(1, 2)); // 3
    }

    {
        using namespace anonymous_concepts_2;

        std::cout << "\n=== Anonymous Concepts 2 ===\n" << std::endl;

        std::println("{}", add(1, 2)); // 3
    }

    {
        using namespace abbreviated_function_template;

        std::cout << "\n=== Abbreviated Function Template ===\n" << std::endl;

        // no constraints
        std::println("{}", add(4, 2));       // 6
        std::println("{}", add(4.0, 2));     // 6
        std::println("{}", add("4"s, "2"s)); // 42 (Oops!)
    }

    {
        using namespace constrained_abbreviated_function_template;

        std::cout << "\n=== Constrained Abbreviated Function Template ===\n" << std::endl;

        std::println("{}", add(4, 2)); // 6

        // std::println("{}", add(4.2, 0));     // error
        // std::println("{}", add("4"s, "2"s)); // error
    }

    {
        using namespace constrained_abbreviated_variadic_function_template;

        std::cout << "\n=== Constrained Abbreviated Variadic Function Template ===\n" << std::endl;

        std::println("{}", add(1, 2, 3)); // 6

        // add(1.0, 2.0, 3.0);                  // error
        // std::println("{}", add("4"s, "2"s)); // error
    }

    {
        using namespace constrained_auto_with_lambdas;

        std::cout << "\n=== Constrained Auto with Lambdas ===\n" << std::endl;

        std::println("{}", sum(1, 2)); // 3
        std::println("{}", twice(2));  // 4
    }
}
