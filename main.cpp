#include <iostream>
#include <type_traits>

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

namespace overloaded_functions
{
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

    template <typename T, typename... Ts> // template parameter pack -> (T1, T2, T3, ...)
    T
    min(T a, Ts... as)                    // function parameter pack -> (T1 a1, T2 a2, T3, a3, ...)
    {
        return min(a, min(as...));        // parameter pack expansion -> a1, a2, a3, ...
    }
} // namespace overloaded_functions

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
        [[maybe_unused]] foo<42, 42.0, false, 'x'> myFoo;
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
}
