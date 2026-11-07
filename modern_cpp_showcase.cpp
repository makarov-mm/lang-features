// ============================================================================
//  modern_cpp_showcase.cpp
//
//  A single-file tour of the features added to C++ during the last ~15 years:
//  C++11, C++14, C++17, C++20 and C++23 (the newest standard that Visual
//  Studio fully targets today).
//
//  Target toolchain
//  ----------------
//    * Visual Studio 2026 (v18.6+) with MSVC Build Tools 14.51 or newer.
//      MSVC 14.51 implements practically all of C++23 (only "consteval
//      propagation" P2564 and "constexpr <cmath>" P0533 are still pending,
//      and this file deliberately does not depend on either).
//
//  How to build (no project files needed)
//  --------------------------------------
//    Developer Command Prompt for VS:
//        cl /std:c++latest /EHsc /W4 /O2 /utf-8 /Zc:__cplusplus modern_cpp_showcase.cpp
//      (/Zc:__cplusplus makes MSVC report the real value of __cplusplus.)
//      (/std:c++23preview also works and restricts you to C++23 exactly.)
//    In the IDE: create an empty C++ project, add this file, and set
//      Project -> Properties -> C/C++ -> Language -> C++ Language Standard =
//      "Preview - Features from the Latest C++ Working Draft (/std:c++latest)".
//
//    It is portable, too:
//        g++ -std=c++23 -O2 modern_cpp_showcase.cpp -lstdc++exp
//        clang++ -std=c++23 -stdlib=libc++ -O2 modern_cpp_showcase.cpp
//
//  About the "#if defined(__cpp_lib_xxx)" guards
//  ---------------------------------------------
//  C++20 standardised *feature-test macros* (header <version>). Every
//  library feature defines a macro such as __cpp_lib_flat_map when it is
//  available. With MSVC + /std:c++latest every guarded block below is active.
//  On other standard libraries (libstdc++, libc++) a few C++23 components are
//  still missing, so the guards keep the file compiling everywhere. Using
//  feature-test macros like this is itself the idiomatic C++20 technique.
//
//  A note on modules (C++20) and "import std;" (C++23)
//  ---------------------------------------------------
//  Modules replace textual #include with compiled, isolated interfaces:
//      export module geometry;            // in geometry.ixx
//      export double area(double r);      // exported declaration
//      import geometry;                   // in a consumer .cpp
//  C++23 adds "import std;" which imports the whole standard library as one
//  pre-built module (much faster than dozens of #includes). MSVC supports it,
//  but it requires the std module to be built by the project/build system,
//  and modules by definition need more than one file, so this single-file
//  demo uses classic #include directives.
//
//  The whole file is plain ASCII on purpose (no encoding surprises in MSVC).
// ============================================================================

#include <version>          // C++20: all feature-test macros in one header

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cmath>
#include <compare>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <future>
#include <initializer_list>
#include <iterator>
#include <latch>
#include <map>
#include <memory>
#include <memory_resource>
#include <new>
#include <mutex>
#include <numbers>
#include <numeric>
#include <optional>
#include <print>
#include <random>
#include <ranges>
#include <regex>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

// __has_include (C++17) lets the preprocessor ask whether a header exists.
#if __has_include(<mdspan>)
#  include <mdspan>
#endif
#if __has_include(<flat_map>)
#  include <flat_map>
#endif
#if __has_include(<flat_set>)
#  include <flat_set>
#endif
#if __has_include(<generator>)
#  include <generator>
#endif
#if __has_include(<stacktrace>)
#  include <stacktrace>
#endif
#if __has_include(<spanstream>)
#  include <spanstream>
#endif
#if __has_include(<syncstream>)
#  include <syncstream>
#  include <iostream>
#endif
#if __has_include(<execution>)
#  include <execution>
#endif

// ---------------------------------------------------------------------------
// Small printing helpers used throughout the demo.
// ---------------------------------------------------------------------------
namespace util {

// Prints a section header.
inline void section(std::string_view title) {
    std::println("");
    std::println("=== {} ===", title);
}

// Joins any input range of formattable elements into "[a, b, c]".
// It uses C++20 concepts (std::ranges::input_range) and std::format.
template <std::ranges::input_range R>
std::string join(R&& r, std::string_view sep = ", ") {
    std::string out = "[";
    bool first = true;
    for (auto&& e : r) {
        if (!first) out += sep;
        first = false;
        out += std::format("{}", e);
    }
    return out + "]";
}

} // namespace util

// ############################################################################
//                                   C++11
// ############################################################################
// C++11 was the "new language" release: move semantics, lambdas, auto,
// variadic templates, a memory model with threads, smart pointers, etc.
namespace cpp11 {

// --- enum class -------------------------------------------------------------
// Scoped, strongly typed enumeration. Enumerators do not leak into the
// enclosing scope and do not implicitly convert to int. The underlying type
// can be specified explicitly (": std::uint8_t").
enum class Color : std::uint8_t { Red = 1, Green = 2, Blue = 4 };

// --- constexpr functions ----------------------------------------------------
// A constexpr function can be evaluated at compile time when its arguments
// are constant expressions. In C++11 the body had to be a single return
// statement (C++14 relaxed this, see below).
constexpr unsigned long long factorial(unsigned n) {
    return n <= 1 ? 1ULL : n * factorial(n - 1);
}

// --- static_assert -----------------------------------------------------------
// Compile-time assertion with a diagnostic message.
static_assert(factorial(5) == 120, "factorial must be computable at compile time");

// --- user-defined literals ----------------------------------------------------
// operator"" lets you attach a suffix to literals: 3.5_km, 42_kg ...
// Since C++23 the form without a space between "" and the suffix is the
// recommended one (the spaced form is deprecated by CWG 2521).
struct Meters { long double value; };
constexpr Meters operator""_km(long double v) { return Meters{v * 1000.0L}; }
constexpr Meters operator""_m(long double v) { return Meters{v}; }

// --- alias templates ("using" instead of typedef) ------------------------------
// "using" can create template aliases, something typedef never could.
template <typename T>
using StringMap = std::map<std::string, T>;

// --- variadic templates + perfect forwarding ------------------------------------
// "typename... Args" accepts any number of template arguments (a parameter
// pack). "Args&&..." are forwarding references; std::forward preserves the
// value category (lvalue / rvalue) of each argument while passing it on.
template <typename T, typename... Args>
std::unique_ptr<T> make(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// sizeof...(Pack) yields the number of elements in a parameter pack.
template <typename... Ts>
constexpr std::size_t count_types(Ts...) { return sizeof...(Ts); }

// Classic C++11 recursive variadic function (C++17 folds simplify this).
inline long long sum_all() { return 0; }
template <typename T, typename... Rest>
long long sum_all(T first, Rest... rest) { return first + sum_all(rest...); }

// --- Move semantics, rule of five, =default / =delete, noexcept -----------------
// An rvalue reference (T&&) binds to temporaries. A move constructor "steals"
// the resources of an expiring object instead of deep-copying them.
// "= default" asks the compiler to generate a special member,
// "= delete" forbids a function. noexcept tells the library that the move
// cannot throw, which allows std::vector to move (instead of copy) when it
// reallocates.
class Buffer {
public:
    Buffer() = default;
    explicit Buffer(std::size_t n) : size_(n), data_(new int[n]{}) {}
    ~Buffer() { delete[] data_; }

    Buffer(const Buffer& other) : size_(other.size_), data_(new int[other.size_]) {
        std::copy(other.data_, other.data_ + size_, data_);
    }
    Buffer& operator=(const Buffer& other) {
        if (this != &other) { Buffer tmp(other); swap(tmp); }
        return *this;
    }
    Buffer(Buffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)), data_(std::exchange(other.data_, nullptr)) {}
    Buffer& operator=(Buffer&& other) noexcept {
        Buffer tmp(std::move(other));
        swap(tmp);
        return *this;
    }
    void swap(Buffer& o) noexcept { std::swap(size_, o.size_); std::swap(data_, o.data_); }
    std::size_t size() const noexcept { return size_; }

private:
    // Non-static data member initializers (NSDMI): default values right at
    // the declaration, used by every constructor that does not override them.
    std::size_t size_ = 0;
    int* data_ = nullptr;   // nullptr: a typed null pointer literal (std::nullptr_t)
};

// --- override / final, delegating & inheriting constructors ---------------------
struct Animal {
    // Delegating constructor: one constructor calls another of the same class.
    Animal() : Animal("unknown") {}
    explicit Animal(std::string n) : name(std::move(n)) {}
    virtual ~Animal() = default;
    // override checks at compile time that a virtual function really overrides
    // something; final forbids further overriding / derivation.
    virtual std::string speak() const { return "..."; }
    std::string name;
};

struct Dog final : Animal {
    using Animal::Animal;                 // inheriting constructors
    std::string speak() const override { return "Woof"; }
};

// --- explicit conversion operators ---------------------------------------------
// "explicit operator bool" allows "if (x)" but forbids "int i = x;".
struct Handle {
    int fd = -1;
    explicit operator bool() const noexcept { return fd >= 0; }
};

// --- [[noreturn]] attribute ------------------------------------------------------
// Attributes use the standard [[...]] syntax since C++11.
[[noreturn]] inline void fatal(const char* msg) { throw std::runtime_error(msg); }

// --- alignas / alignof ------------------------------------------------------------
struct alignas(32) SimdBlock { float lanes[8]; };
static_assert(alignof(SimdBlock) == 32);

// --- thread_local -----------------------------------------------------------------
// Each thread gets its own instance of a thread_local variable.
thread_local int tls_counter = 0;

void run() {
    util::section("C++11");

    // auto: type deduction from the initializer.
    auto answer = 42;                                   // int
    auto pi = 3.14159;                                  // double
    // decltype: the declared type of an expression.
    decltype(answer) copy = answer;                     // int
    std::println("auto/decltype: {} {} {}", answer, pi, copy);

    // Uniform (brace) initialization and std::initializer_list.
    std::vector<int> v{5, 3, 8, 1, 9, 2};
    std::map<std::string, int> ages{{"Ann", 31}, {"Bob", 27}};

    // Range-based for loop.
    int total = 0;
    for (int x : v) total += x;
    std::println("range-for sum = {}", total);

    // Lambda expressions: [captures](params) -> ret { body }.
    int threshold = 4;
    auto above = std::count_if(v.begin(), v.end(), [threshold](int x) { return x > threshold; });
    std::println("lambda count_if(>{}) = {}", threshold, above);

    // Sorting with a lambda comparator.
    std::sort(v.begin(), v.end(), [](int a, int b) { return a > b; });
    std::println("sorted desc = {}", util::join(v));

    // Trailing return types: "auto f() -> T", useful when the type depends on
    // the parameters.
    auto add = [](int a, double b) -> double { return a + b; };
    std::println("trailing return: {}", add(2, 0.5));

    // enum class
    Color c = Color::Green;
    std::println("enum class underlying value = {}", static_cast<int>(c));

    // constexpr + user-defined literals
    constexpr auto dist = 1.5_km;
    constexpr auto shortDist = 250.0_m;
    std::println("user-defined literals: {} m and {} m", static_cast<double>(dist.value),
                 static_cast<double>(shortDist.value));

    // Raw string literals: no escaping, ideal for regex, JSON, paths.
    const char* raw = R"(C:\temp\new "quoted" \n not an escape)";
    std::println("raw string: {}", raw);

    // Unicode string literals: u8"..." (UTF-8), u"..." (UTF-16), U"..." (UTF-32).
    // \u03C0 is the Greek letter pi.
    std::u16string utf16 = u"pi=\u03C0";
    std::u32string utf32 = U"pi=\u03C0";
    std::println("char16_t/char32_t strings: {} and {} code units", utf16.size(), utf32.size());

    // Smart pointers: unique_ptr (single owner), shared_ptr (reference
    // counted), weak_ptr (non-owning observer that breaks cycles).
    std::unique_ptr<Dog> dog = make<Dog>("Rex");
    std::shared_ptr<Animal> shared = std::make_shared<Dog>("Fido");
    std::weak_ptr<Animal> weak = shared;
    std::println("unique_ptr: {} says {}, shared use_count={}, weak expired={}",
                 dog->name, dog->speak(), shared.use_count(), weak.expired());
    if (auto locked = weak.lock()) std::println("weak_ptr locked: {}", locked->name);

    // Move semantics
    Buffer a(1000);
    Buffer b = std::move(a);  // no allocation, just pointer stealing
    std::println("move: a.size={}, b.size={}", a.size(), b.size());

    // Variadic templates
    std::println("variadic: sum_all={}, count_types={}", sum_all(1, 2, 3, 4), count_types(1, 'a', 2.0));

    // Alias templates
    StringMap<double> prices{{"apple", 0.5}};
    std::println("alias template map size = {}", prices.size());

    // explicit operator bool
    Handle h{3};
    if (h) std::println("explicit operator bool: handle valid");

    // std::array: fixed-size container with value semantics.
    std::array<int, 4> arr{1, 2, 3, 4};
    std::println("std::array size={} back={}", arr.size(), arr.back());

    // std::tuple and std::tie
    std::tuple<int, std::string, double> t{1, "one", 1.0};
    int n; std::string s; double d;
    std::tie(n, s, d) = t;
    std::println("tuple/tie: {} {} {}", n, s, d);

    // std::unordered_map: hash table.
    std::unordered_map<std::string, int> wordCount;
    for (auto w : {"a", "b", "a", "c", "a"}) ++wordCount[w];
    std::println("unordered_map: count(a) = {}", wordCount["a"]);

    // std::function: type-erased callable wrapper.
    std::function<int(int)> square = [](int x) { return x * x; };
    std::println("std::function: {}", square(7));

    // <chrono>: type-safe durations and clocks.
    auto t0 = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - t0);
    std::println("chrono: elapsed >= 0us: {}", elapsed.count() >= 0);

    // <random>: engines + distributions instead of rand().
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dice(1, 6);
    std::println("random dice roll = {}", dice(rng));

    // <regex>
    std::regex date(R"((\d{4})-(\d{2})-(\d{2}))");
    std::smatch m;
    std::string text = "Released on 2011-08-12.";
    if (std::regex_search(text, m, date)) std::println("regex: year={} month={}", m[1].str(), m[2].str());

    // <type_traits>: compile-time type introspection.
    static_assert(std::is_integral<int>::value && !std::is_integral<double>::value);

    // Concurrency: std::thread, std::mutex, std::lock_guard, std::atomic,
    // std::async/std::future, std::condition_variable, thread_local.
    std::atomic<int> counter{0};
    std::mutex mtx;
    std::vector<int> collected;
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back([&, i] {
            ++tls_counter;                       // per-thread copy
            counter.fetch_add(1, std::memory_order_relaxed);
            std::lock_guard<std::mutex> lock(mtx);
            collected.push_back(i * tls_counter);
        });
    }
    for (auto& w : workers) w.join();
    std::println("threads: atomic counter={}, collected {} items", counter.load(), collected.size());

    std::future<unsigned long long> fut = std::async(std::launch::async, [] { return factorial(10) % 1000; });
    std::println("std::async result = {}", fut.get());

    std::condition_variable cv;
    bool ready = false;
    std::thread producer([&] {
        { std::lock_guard<std::mutex> lk(mtx); ready = true; }
        cv.notify_one();
    });
    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [&] { return ready; });
    }
    producer.join();
    std::println("condition_variable: ready={}", ready);

    try { fatal("demo of [[noreturn]]"); }
    catch (const std::exception& e) { std::println("caught: {}", e.what()); }

    // long long and type-safe nullptr were also new in C++11.
    long long big = 9'000'000'000LL;
    int* nothing = nullptr;
    std::println("long long={}, nullptr is null: {}", big, nothing == nullptr);
}

} // namespace cpp11

// ############################################################################
//                                   C++14
// ############################################################################
// C++14 was a "polish" release that made C++11 features more convenient.
namespace cpp14 {

// --- Return type deduction -------------------------------------------------
// Ordinary functions can use "auto" as return type; the compiler deduces it
// from the return statements.
auto make_greeting(std::string_view name) { return std::string("Hello, ") + std::string(name); }

// --- decltype(auto) -----------------------------------------------------------
// Deduces the type exactly as decltype would, preserving references. Useful
// for forwarding wrappers that must return exactly what the callee returns.
template <typename Container>
decltype(auto) first_element(Container& c) { return c[0]; }   // returns a reference

// --- Relaxed constexpr ----------------------------------------------------------
// constexpr functions may now contain loops, local variables, and multiple
// statements.
constexpr int count_primes_below(int limit) {
    int count = 0;
    for (int n = 2; n < limit; ++n) {
        bool prime = true;
        for (int d = 2; d * d <= n; ++d)
            if (n % d == 0) { prime = false; break; }
        if (prime) ++count;
    }
    return count;
}
static_assert(count_primes_below(100) == 25);

// --- Variable templates ------------------------------------------------------------
// A variable parameterised by a type (used by the library for *_v traits).
template <typename T>
constexpr T tau = T(6.283185307179586476925286766559L);

// --- [[deprecated]] attribute -------------------------------------------------------
// Using a deprecated entity produces a compiler warning with the message.
[[deprecated("use make_greeting instead")]] inline std::string old_greeting() { return "hi"; }

void run() {
    util::section("C++14");

    // Generic lambdas: "auto" parameters make the lambda's operator() a template.
    auto plus = [](auto a, auto b) { return a + b; };
    std::println("generic lambda: {} {}", plus(1, 2), plus(std::string("a"), std::string("b")));

    // Init-capture (generalized lambda capture): create a new variable in the
    // capture list, e.g. to *move* a unique_ptr into a lambda.
    auto ptr = std::make_unique<int>(99);                 // std::make_unique is C++14 too
    auto owner = [p = std::move(ptr), calls = 0]() mutable { ++calls; return *p + calls; };
    std::println("init-capture: {} (ptr now empty: {})", owner(), ptr == nullptr);

    // Return type deduction
    std::println("auto return: {}", make_greeting("C++14"));

    // decltype(auto) preserved the reference, so we can assign through it.
    std::vector<int> v{1, 2, 3};
    first_element(v) = 100;
    std::println("decltype(auto) reference: v[0] = {}", v[0]);

    // Binary literals and digit separators (the apostrophe).
    int mask = 0b1010'1010;
    long population = 8'100'000'000 / 1000;
    std::println("binary literal={} digit separators={}", mask, population);

    // Variable template
    std::println("variable template tau<float>={} tau<double>={}", tau<float>, tau<double>);

    // Relaxed constexpr
    constexpr int primes = count_primes_below(1000);
    std::println("relaxed constexpr: primes below 1000 = {}", primes);

    // Standard literals for chrono durations and std::string.
    using namespace std::chrono_literals;
    using namespace std::string_literals;
    auto timeout = 1500ms;
    auto total = 2s + timeout;
    auto str = "literal std::string"s;                    // type is std::string
    std::println("chrono literals: {} ms, {} (size {})", total.count(), str, str.size());

    // std::exchange: replace a value and return the old one.
    int state = 1;
    int previous = std::exchange(state, 2);
    std::println("std::exchange: previous={} now={}", previous, state);

    // std::integer_sequence / make_index_sequence: compile-time index packs,
    // typically used to "unpack" tuples. (For brevity the helper below uses a
    // C++20 template lambda and a C++17 fold expression.)
    auto print_tuple = []<typename Tuple, std::size_t... I>(const Tuple& t, std::index_sequence<I...>) {
        std::string out;
        ((out += std::format("{}{}", I == 0 ? "" : ", ", std::get<I>(t))), ...);
        return out;
    };
    auto tup = std::make_tuple(1, "two", 3.0);
    std::println("index_sequence: ({})", print_tuple(tup, std::make_index_sequence<3>{}));

    // Heterogeneous (transparent) lookup: std::less<> allows find() with any
    // comparable type (here string_view) without building a std::string.
    std::set<std::string, std::less<>> names{"alpha", "beta"};
    std::string_view key = "beta";
    std::println("transparent comparator find: {}", names.find(key) != names.end());

    // std::shared_timed_mutex (C++14) / std::shared_mutex (C++17):
    // many readers or one writer.
    std::shared_mutex rw;
    { std::shared_lock read_lock(rw); }
    { std::unique_lock write_lock(rw); }
    std::println("shared (reader/writer) mutex: ok");
}

} // namespace cpp14

// ############################################################################
//                                   C++17
// ############################################################################
// C++17 brought vocabulary types (optional, variant, any, string_view),
// structured bindings, if constexpr, fold expressions, filesystem, etc.

// Nested namespace definition: "namespace a::b::c { }" instead of three blocks.
namespace cpp17::detail::geometry {
struct Point { double x, y; };
}

namespace cpp17 {

// --- inline variables -------------------------------------------------------
// A variable defined in a header can now be "inline": one definition shared
// across all translation units (no more separate .cpp definition).
inline constexpr std::string_view version_name = "C++17";

// --- Fold expressions ----------------------------------------------------------
// (... op pack) / (pack op ...) apply a binary operator over a whole pack.
template <typename... Ts>
auto sum(Ts... xs) { return (xs + ... + 0); }

template <typename... Ts>
bool all_true(Ts... xs) { return (... && xs); }

// --- if constexpr ------------------------------------------------------------------
// Compile-time branch: the discarded branch is not instantiated, so it may
// contain code that would not compile for the current type.
template <typename T>
std::string describe(const T& value) {
    if constexpr (std::is_integral_v<T>)
        return std::format("integer {}", value);
    else if constexpr (std::is_floating_point_v<T>)
        return std::format("floating {:.3f}", value);
    else
        return std::format("something else: {}", value);
}

// --- auto non-type template parameters ---------------------------------------------
// template <auto N>: the NTTP's type is deduced from the argument.
template <auto Value>
constexpr auto constant = Value;

// --- [[nodiscard]], [[maybe_unused]], [[fallthrough]] ---------------------------------
[[nodiscard]] inline int compute_important() { return 7; }

inline int classify(int n) {
    int score = 0;
    switch (n) {
        case 0:
            score += 1;
            [[fallthrough]];        // intentional fall-through, no warning
        case 1:
            score += 10;
            break;
        default:
            break;
    }
    return score;
}

// --- Class template argument deduction (CTAD) ---------------------------------------
// The compiler deduces class template arguments from constructor arguments:
//     std::pair p{1, 2.0};   instead of   std::pair<int, double> p{1, 2.0};
template <typename T>
struct Box {
    T value;
    explicit Box(T v) : value(std::move(v)) {}
};
// User-defined deduction guide: string literals become std::string.
Box(const char*) -> Box<std::string>;

// --- The "overloaded" idiom for std::visit -------------------------------------------
// Inherit from several lambdas and pull in all their operator() overloads with
// a pack-expanded using-declaration (C++17). Since C++20 aggregates support
// CTAD directly, so no deduction guide is needed.
template <typename... Fs>
struct overloaded : Fs... { using Fs::operator()...; };

// --- constexpr lambda & capture of *this ------------------------------------------------
struct Accumulator {
    int base = 10;
    auto adder() const {
        // [*this] captures a *copy* of the object, so the lambda remains valid
        // even if the original object dies (important for async callbacks).
        return [*this](int x) { return base + x; };
    }
};

// --- Guaranteed copy elision ---------------------------------------------------------------
// Returning a prvalue never copies or moves; the object is constructed in place.
// This even works for non-movable types.
struct NonMovable {
    explicit NonMovable(int v) : value(v) {}
    NonMovable(const NonMovable&) = delete;
    NonMovable(NonMovable&&) = delete;
    int value;
};
inline NonMovable make_non_movable() { return NonMovable{5}; }

void run() {
    util::section(version_name);

    // Structured bindings: unpack tuples, pairs, arrays, and plain structs.
    std::map<std::string, int> inventory{{"apples", 3}, {"pears", 7}};
    for (const auto& [fruit, qty] : inventory)
        std::println("structured binding: {} -> {}", fruit, qty);

    detail::geometry::Point p{3.0, 4.0};
    auto [px, py] = p;
    std::println("struct binding + nested namespace: hypot = {}", std::hypot(px, py));

    // if with initializer (also: switch with initializer).
    if (auto it = inventory.find("pears"); it != inventory.end())
        std::println("if-init: found pears = {}", it->second);
    switch (int code = classify(0); code) {
        case 11: std::println("switch-init + [[fallthrough]]: code = {}", code); break;
        default: break;
    }

    // Fold expressions
    std::println("fold: sum={}, all_true={}", sum(1, 2, 3, 4.5), all_true(true, 1, 'x'));

    // if constexpr
    std::println("if constexpr: {} | {} | {}", describe(42), describe(2.5), describe("text"));

    // auto NTTP
    std::println("template<auto>: {} {}", constant<42>, constant<'Z'>);

    // [[nodiscard]] / [[maybe_unused]]
    [[maybe_unused]] int unused_but_ok = 0;
    int important = compute_important();
    std::println("[[nodiscard]] result used: {}", important);

    // CTAD
    std::pair pr{1, 2.5};              // std::pair<int, double>
    std::vector vec{1, 2, 3};          // std::vector<int>
    Box bx("hello");                   // Box<std::string> via deduction guide
    std::println("CTAD: pair.second={} vec.size={} Box<std::string>={}", pr.second, vec.size(), bx.value);

    // std::optional: maybe-a-value.
    auto parse_positive = [](int x) -> std::optional<int> {
        if (x > 0) return x;
        return std::nullopt;
    };
    std::println("optional: {} / value_or={}", parse_positive(5).value(), parse_positive(-1).value_or(0));

    // std::variant: type-safe union + std::visit.
    using Shape = std::variant<int, double, std::string>;
    std::vector<Shape> shapes{3, 2.5, std::string("circle")};
    for (const auto& sh : shapes) {
        std::visit(overloaded{
            [](int i) { std::println("variant holds int {}", i); },
            [](double d) { std::println("variant holds double {}", d); },
            [](const std::string& s) { std::println("variant holds string {}", s); },
        }, sh);
    }
    std::println("holds_alternative<double>(shapes[1]) = {}", std::holds_alternative<double>(shapes[1]));

    // std::any: holds a value of any copyable type.
    std::any anything = 3.14;
    anything = std::string("now a string");
    std::println("std::any: {}", std::any_cast<std::string>(anything));

    // std::string_view: non-owning view of a string; no allocation.
    using namespace std::string_view_literals;
    std::string_view sv = "Hello, string_view"sv;
    std::println("string_view: substr='{}' find={}", sv.substr(7), sv.find("view"));

    // std::apply calls a function with a tuple's elements as arguments;
    // std::invoke calls anything callable (functions, members, lambdas).
    auto tup = std::make_tuple(2, 3);
    std::println("apply: {}, invoke: {}", std::apply([](int a, int b) { return a * b; }, tup),
                 std::invoke(&Accumulator::base, Accumulator{}));

    // constexpr lambda and capture *this
    constexpr auto cube = [](int x) constexpr { return x * x * x; };
    static_assert(cube(3) == 27);
    auto add10 = Accumulator{}.adder();
    std::println("constexpr lambda: {}, [*this] capture: {}", cube(4), add10(5));

    // Guaranteed copy elision with a non-movable type.
    NonMovable nm = make_non_movable();
    std::println("guaranteed copy elision: {}", nm.value);

    // std::filesystem: portable paths, directories, file status.
    std::filesystem::path path = "projects/demo/src/main.cpp";
    std::println("filesystem: filename={} stem={} ext={} parent={}", path.filename().string(),
                 path.stem().string(), path.extension().string(), path.parent_path().string());

    // <charconv>: fastest locale-independent number <-> text conversion.
    const char digits[] = "12345.678";
    double parsed = 0;
    auto [ptr, ec] = std::from_chars(digits, digits + std::strlen(digits), parsed);
    char out[32]{};
    auto res = std::to_chars(out, out + sizeof out, 255, 16);
    std::println("charconv: from_chars={} ok={} to_chars(hex)={}", parsed, ec == std::errc{},
                 std::string_view(out, res.ptr));
    (void)ptr;

    // Map node handles and new insertion helpers.
    std::map<int, std::string> m1{{1, "one"}, {2, "two"}};
    std::map<int, std::string> m2{{3, "three"}};
    m1.try_emplace(1, "ignored");            // no-op: key exists, value not even constructed
    m1.insert_or_assign(2, "TWO");           // overwrite
    auto node = m1.extract(1);               // detach a node without reallocation
    node.key() = 10;                         // keys can be modified on extracted nodes
    m2.insert(std::move(node));
    m2.merge(m1);                            // splice all nodes
    std::println("map extract/merge: m2 has {} items, m2[10]={}", m2.size(), m2[10]);

    // std::scoped_lock: lock several mutexes at once, deadlock-free.
    std::mutex a, b;
    { std::scoped_lock lock(a, b); }

    // Math helpers: std::gcd, std::lcm, std::clamp.
    std::println("gcd={} lcm={} clamp={}", std::gcd(84, 36), std::lcm(4, 6), std::clamp(15, 0, 10));

    // std::byte: a distinct type for raw memory (not a character, not a number).
    std::byte bt{0b1111'0000};
    bt >>= 4;
    std::println("std::byte: {}", std::to_integer<int>(bt));

    // std::size / std::data / std::empty free functions work for arrays too.
    int raw[] = {1, 2, 3};
    std::println("std::size(raw) = {}", std::size(raw));

    // std::sample, std::reduce (an order-free std::accumulate).
    std::vector<int> population(100);
    std::iota(population.begin(), population.end(), 1);
    std::vector<int> picked;
    std::sample(population.begin(), population.end(), std::back_inserter(picked), 5, std::mt19937{7});
    std::println("std::sample picked {} items, reduce = {}", picked.size(),
                 std::reduce(population.begin(), population.end()));

    // Parallel algorithms: pass an execution policy (seq, par, par_unseq,
    // and unseq since C++20) to run standard algorithms on multiple cores.
#if defined(__cpp_lib_execution)
    std::vector<double> big(1'000'000, 0.5);
    double s = std::reduce(std::execution::par, big.begin(), big.end(), 0.0);
    std::sort(std::execution::par_unseq, population.begin(), population.end(), std::greater<>{});
    std::println("parallel reduce = {}, par_unseq sort front = {}", s, population.front());
#endif

    // Polymorphic memory resources (std::pmr): pluggable allocators chosen at
    // runtime. Here the vector allocates from a stack buffer.
    std::array<std::byte, 1024> storage{};
    std::pmr::monotonic_buffer_resource pool(storage.data(), storage.size());
    std::pmr::vector<int> pmr_vec(&pool);
    for (int i = 0; i < 10; ++i) pmr_vec.push_back(i * i);
    std::println("pmr vector from stack buffer: {}", util::join(pmr_vec));

    // std::not_fn negates a predicate.
    auto is_even = [](int x) { return x % 2 == 0; };
    std::println("not_fn: odd count = {}", std::count_if(vec.begin(), vec.end(), std::not_fn(is_even)));

    // Hexadecimal floating point literals: exact binary representation.
    double hexf = 0x1.8p1;  // 1.5 * 2^1 = 3.0
    std::println("hex float literal 0x1.8p1 = {}", hexf);

    // Cache-line size constants for avoiding false sharing.
#if defined(__cpp_lib_hardware_interference_size)
    std::println("hardware_destructive_interference_size = {}", std::hardware_destructive_interference_size);
#endif
}

} // namespace cpp17

// ############################################################################
//                                   C++20
// ############################################################################
// C++20 is the second huge release: concepts, ranges, coroutines, modules,
// the spaceship operator, std::format, std::span, designated initializers...
namespace cpp20 {

// --- Concepts -------------------------------------------------------------
// A concept is a named compile-time predicate on types. It replaces SFINAE
// tricks with readable constraints and gives far better error messages.
template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// requires-expression: lists operations a type must support.
template <typename T>
concept Shape = requires(const T& s) {
    { s.area() } -> std::convertible_to<double>;
    { s.name() } -> std::convertible_to<std::string_view>;
};

// Constrained template (requires-clause).
template <typename T>
    requires Numeric<T>
T square(T x) { return x * x; }

// Abbreviated function template: a constrained "auto" parameter.
double total_area(const Shape auto&... shapes) { return (shapes.area() + ... + 0.0); }

// Concept subsumption: the more constrained overload wins.
template <typename T> concept Animal = requires(T a) { a.sound(); };
template <typename T> concept Pet = Animal<T> && requires(T a) { a.owner(); };
std::string_view kind(const Animal auto&) { return "animal"; }
std::string_view kind(const Pet auto&) { return "pet"; }

struct Wolf { void sound() const {} };
struct Cat { void sound() const {} void owner() const {} };

struct Circle {
    double r;
    double area() const { return std::numbers::pi * r * r; }  // <numbers>: math constants
    std::string_view name() const { return "circle"; }
};
struct Rect {
    double w, h;
    double area() const { return w * h; }
    std::string_view name() const { return "rect"; }
};

// --- Three-way comparison (spaceship) operator <=> --------------------------------
// "= default" generates <, <=, >, >= and (with defaulted ==) also == and !=,
// comparing members lexicographically.
struct Version {
    int major = 0, minor = 0, patch = 0;
    auto operator<=>(const Version&) const = default;
};

// --- Designated initializers ---------------------------------------------------------
struct WindowConfig {
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    std::string_view title = "untitled";
};

// --- consteval and constinit -----------------------------------------------------------
// consteval: an "immediate function" that MUST be evaluated at compile time.
consteval int compile_time_only(int x) { return x * 100; }
// constinit: the variable must be initialized at compile time (prevents the
// "static initialization order fiasco") but it stays mutable.
constinit int global_counter = compile_time_only(1);

// --- constexpr virtual, constexpr try/catch, constexpr dynamic allocation ------------
struct Base {
    virtual ~Base() = default;
    constexpr virtual int id() const { return 1; }
};
struct Derived : Base {
    constexpr int id() const override { return 2; }
};
constexpr int virtual_at_compile_time() {
    Derived d;
    const Base& b = d;
    return b.id();
}
static_assert(virtual_at_compile_time() == 2);

// std::vector and std::string are usable inside constant evaluation (memory
// must be released before evaluation ends).
constexpr int sum_of_squares_constexpr(int n) {
    std::vector<int> v;
    for (int i = 1; i <= n; ++i) v.push_back(i * i);
    return std::accumulate(v.begin(), v.end(), 0);
}
static_assert(sum_of_squares_constexpr(4) == 30);

// std::is_constant_evaluated(): detect compile-time evaluation (superseded by
// "if consteval" in C++23, see below).
constexpr const char* where_am_i() {
    return std::is_constant_evaluated() ? "compile time" : "run time";
}

// --- Class types as non-type template parameters ------------------------------------------
// A "structural" literal class can be used as a template argument; a common use is
// passing string literals to templates.
template <std::size_t N>
struct FixedString {
    char data[N]{};
    constexpr FixedString(const char (&str)[N]) { std::copy_n(str, N, data); }
    constexpr std::string_view view() const { return {data, N - 1}; }
};
template <FixedString Name>
struct Tag {
    static constexpr std::string_view name() { return Name.view(); }
};

// --- explicit(bool) ---------------------------------------------------------------------------
// Conditionally explicit constructor: implicit only for "safe" conversions.
template <typename T>
struct Wrapper {
    template <typename U>
    explicit(!std::is_convertible_v<U, T>) Wrapper(U u) : value(static_cast<T>(u)) {}
    T value;
};

// --- [[no_unique_address]] ------------------------------------------------------------------
// An empty member may occupy no storage (MSVC honors [[msvc::no_unique_address]];
// the standard attribute is accepted and portable).
struct Empty {};
struct Compact {
    [[no_unique_address]] Empty policy;
    int value;
};

// --- Coroutines ------------------------------------------------------------------------------
// A function containing co_yield / co_await / co_return is a coroutine. Its
// state lives in a heap-allocated frame, so it can be suspended and resumed.
// The return type's nested promise_type defines the behavior. Below is a
// hand-written lazy generator (C++23 provides std::generator, shown later).
template <typename T>
class Generator {
public:
    struct promise_type {
        T current{};
        Generator get_return_object() { return Generator{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_always initial_suspend() noexcept { return {}; }   // start suspended (lazy)
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T v) noexcept { current = std::move(v); return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { throw; }
    };

    struct sentinel {};
    class iterator {
    public:
        explicit iterator(std::coroutine_handle<promise_type> h) : h_(h) {}
        const T& operator*() const { return h_.promise().current; }
        iterator& operator++() { h_.resume(); return *this; }
        bool operator==(sentinel) const { return h_.done(); }
    private:
        std::coroutine_handle<promise_type> h_;
    };

    explicit Generator(std::coroutine_handle<promise_type> h) : h_(h) {}
    Generator(Generator&& o) noexcept : h_(std::exchange(o.h_, {})) {}
    ~Generator() { if (h_) h_.destroy(); }

    iterator begin() { h_.resume(); return iterator{h_}; }
    sentinel end() { return {}; }

private:
    std::coroutine_handle<promise_type> h_;
};

Generator<unsigned long long> fibonacci(int count) {
    unsigned long long a = 0, b = 1;
    for (int i = 0; i < count; ++i) {
        co_yield a;                       // suspend and hand a value to the caller
        a = std::exchange(b, a + b);
    }
}

// A lazily started task type that supports co_await and co_return, using
// "symmetric transfer" (await_suspend returning a coroutine_handle) so that
// awaiting chains never grow the native stack.
template <typename T>
class Lazy {
public:
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;

    struct promise_type {
        std::optional<T> value;
        std::coroutine_handle<> continuation = std::noop_coroutine();

        Lazy get_return_object() { return Lazy{handle::from_promise(*this)}; }
        std::suspend_always initial_suspend() noexcept { return {}; }
        struct FinalAwaiter {
            bool await_ready() noexcept { return false; }
            std::coroutine_handle<> await_suspend(handle h) noexcept { return h.promise().continuation; }
            void await_resume() noexcept {}
        };
        FinalAwaiter final_suspend() noexcept { return {}; }
        void return_value(T v) { value = std::move(v); }
        void unhandled_exception() { std::terminate(); }
    };

    explicit Lazy(handle h) : h_(h) {}
    Lazy(Lazy&& o) noexcept : h_(std::exchange(o.h_, {})) {}
    ~Lazy() { if (h_) h_.destroy(); }

    // Awaitable interface: lets another coroutine "co_await" this task.
    bool await_ready() const noexcept { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaiting) noexcept {
        h_.promise().continuation = awaiting;
        return h_;                                 // start the awaited coroutine
    }
    T await_resume() { return std::move(*h_.promise().value); }

    // Synchronous driver for the outermost task.
    T get() {
        h_.resume();
        return std::move(*h_.promise().value);
    }

private:
    handle h_;
};

Lazy<int> load_value(int x) { co_return x * 2; }
Lazy<int> combined() {
    int a = co_await load_value(10);
    int b = co_await load_value(11);
    co_return a + b;
}

// --- Formatting user types with std::format ---------------------------------------------------
struct Vec2 { double x, y; };

} // namespace cpp20

// Specializing std::formatter makes a type usable with std::format/std::print.
template <>
struct std::formatter<cpp20::Vec2> : std::formatter<std::string> {
    auto format(const cpp20::Vec2& v, std::format_context& ctx) const {
        return std::formatter<std::string>::format(std::format("Vec2({:.1f}, {:.1f})", v.x, v.y), ctx);
    }
};

namespace cpp20 {

void log_here(std::string_view msg, std::source_location loc = std::source_location::current()) {
    // std::source_location replaces __FILE__/__LINE__ macros with a real type.
    std::println("source_location: '{}' at line {} in {}", msg, loc.line(), loc.function_name());
}

void run() {
    util::section("C++20");

    // Concepts
    std::println("concepts: square(7)={} square(1.5)={}", square(7), square(1.5));
    std::println("abbreviated template: total area = {:.3f}", total_area(Circle{1.0}, Rect{2, 3}));
    std::println("concept subsumption: Wolf is {}, Cat is {}", kind(Wolf{}), kind(Cat{}));
    static_assert(Shape<Circle> && !Shape<int>);

    // Ranges library: composable, lazy views with the pipe syntax, and
    // range-based algorithms that take whole containers plus projections.
    std::vector<int> numbers{5, 12, 7, 3, 18, 21, 4, 9};
    auto even_squares = numbers
                      | std::views::filter([](int n) { return n % 2 == 0; })
                      | std::views::transform([](int n) { return n * n; });
    std::println("ranges filter|transform: {}", util::join(even_squares));
    std::println("views::iota|take|reverse: {}", util::join(std::views::iota(1, 100) | std::views::take(5) | std::views::reverse));

    std::ranges::sort(numbers);
    std::println("ranges::sort: {}", util::join(numbers));

    struct Person { std::string name; int age; };
    std::vector<Person> people{{"Zoe", 30}, {"Adam", 41}, {"Mia", 25}};
    std::ranges::sort(people, {}, &Person::age);       // projection: sort by member
    std::println("ranges::sort with projection: youngest = {}", people.front().name);
    auto oldest = std::ranges::max_element(people, {}, &Person::age);
    std::println("ranges::max_element by age: {}", oldest->name);

    std::map<std::string, int> scores{{"a", 1}, {"b", 2}};
    std::println("views::keys={} views::values={}", util::join(std::views::keys(scores)), util::join(std::views::values(scores)));

    std::string csv = "red,green,blue";
    std::vector<std::string> parts;
    for (auto part : csv | std::views::split(','))
        parts.emplace_back(part.begin(), part.end());
    std::println("views::split: {}", util::join(parts));

    // Spaceship operator
    Version v1{1, 2, 3}, v2{1, 3, 0};
    std::println("<=>: v1<v2={} v1==v1={} ordering is less={}", v1 < v2, v1 == v1, (v1 <=> v2) < 0);

    // Designated initializers (in declaration order; omitted members use defaults)
    WindowConfig cfg{.width = 1920, .height = 1080, .title = "Showcase"};
    std::println("designated init: {}x{} fullscreen={} title={}", cfg.width, cfg.height, cfg.fullscreen, cfg.title);

    // Parenthesized aggregate initialization (works like braces, but allows narrowing).
    Rect r(2.0, 5.0);
    std::println("aggregate paren-init area = {}", r.area());

    // consteval / constinit / constexpr containers / is_constant_evaluated
    constexpr int ct = compile_time_only(7);
    ++global_counter;
    constexpr const char* when = where_am_i();
    std::println("consteval={} constinit global={} constexpr vector={} evaluated at {}",
                 ct, global_counter, sum_of_squares_constexpr(10), when);

    // Template lambdas: explicit template parameter list on lambdas.
    auto sizeof_sum = []<typename... Ts>(Ts...) { return (sizeof(Ts) + ... + 0); };
    std::println("template lambda: sizeof sum = {}", sizeof_sum(1, 2.0, 'c'));

    // Lambdas in unevaluated contexts + default-constructible stateless lambdas.
    using Descending = decltype([](int a, int b) { return a > b; });
    std::set<int, Descending> desc{3, 1, 4, 1, 5};
    std::println("lambda as comparator type: {}", util::join(desc));

    // Range-for with initializer.
    for (int index = 0; auto c : std::string_view("abc")) std::println("range-for init: {}:{}", index++, c);

    // using enum: bring enumerators into scope (handy in switches).
    enum class Op { Add, Sub };
    auto apply_op = [](Op op, int a, int b) {
        switch (op) {
            using enum Op;
            case Add: return a + b;
            case Sub: return a - b;
        }
        return 0;
    };
    std::println("using enum: {}", apply_op(Op::Sub, 10, 3));

    // [[likely]] / [[unlikely]]: branch-probability hints for the optimizer.
    int value = 42;
    if (value > 0) [[likely]] std::println("[[likely]] branch taken");
    else [[unlikely]] std::println("never");

    // Class-type NTTP / string literal as template argument
    std::println("FixedString NTTP: Tag name = {}", Tag<"sensor-7">::name());

    // explicit(bool)
    Wrapper<double> w = 3;               // int -> double: implicit allowed
    std::println("explicit(bool): {}", w.value);

    // [[no_unique_address]]
    std::println("[[no_unique_address]]: sizeof(Compact)={}", sizeof(Compact));

    // Coroutines
    std::string fib;
    for (auto f : fibonacci(10)) fib += std::format("{} ", f);
    std::println("coroutine generator (co_yield): {}", fib);
    std::println("coroutine task (co_await/co_return): {}", combined().get());

    // std::format with a custom formatter; format specs like {:>8} {:08.3f} {:#x}.
    std::println("std::format: {:>8}|{:<6}|{:^7}|{:08.3f}|{:#x}|{:b}", "right", "left", "mid", 3.14159, 255, 5);
    std::println("custom formatter: {}", Vec2{1.25, -2.5});

    // std::span: a non-owning view over contiguous memory (array, vector, C array).
    int raw[] = {1, 2, 3, 4, 5, 6};
    std::span<int> all(raw);
    std::span<int, 3> fixed = all.first<3>();
    std::println("span: size={} last2={} fixed extent={}", all.size(), util::join(all.last(2)), fixed.extent);
    std::println("std::ssize (signed size) = {}", std::ssize(raw));

    // source_location
    log_here("hello");

    // <bit>: bit manipulation utilities and std::bit_cast for type punning.
    std::println("<bit>: popcount(255)={} has_single_bit(64)={} bit_width(100)={} rotl(1,3)={} countr_zero(8)={}",
                 std::popcount(255u), std::has_single_bit(64u), std::bit_width(100u), std::rotl(1u, 3), std::countr_zero(8u));
    float f = 1.0f;
    std::println("bit_cast<uint32_t>(1.0f) = {:#010x}, endian::native is little: {}",
                 std::bit_cast<std::uint32_t>(f), std::endian::native == std::endian::little);

    // <numbers>
    std::println("<numbers>: pi={:.6f} e={:.6f} sqrt2={:.6f}", std::numbers::pi, std::numbers::e, std::numbers::sqrt2);

    // Container conveniences: contains, starts_with/ends_with, std::erase_if.
    std::string url = "https://isocpp.org";
    std::vector<int> nums{1, 2, 3, 4, 5, 6};
    std::erase_if(nums, [](int n) { return n % 2 == 0; });
    std::println("contains={} starts_with={} ends_with={} erase_if -> {}",
                 scores.contains("a"), url.starts_with("https"), url.ends_with(".org"), util::join(nums));

    // Safe integer comparisons between signed and unsigned values.
    int neg = -1; unsigned int big = 1;
    std::println("cmp_less(-1, 1u)={} (built-in '<' would say {}), in_range<uint8_t>(300)={}",
                 std::cmp_less(neg, big), static_cast<unsigned>(neg) < big, std::in_range<std::uint8_t>(300));

    // std::midpoint, std::lerp (overflow-safe / exact).
    std::println("midpoint(INT_MAX-1, INT_MAX)={} lerp(0,10,0.25)={}",
                 std::midpoint(2147483646, 2147483647), std::lerp(0.0, 10.0, 0.25));

    // std::to_array, make_shared for arrays, remove_cvref, type_identity, bind_front
    auto arr = std::to_array({3, 1, 2});
    auto shared_arr = std::make_shared<int[]>(4);
    static_assert(std::is_same_v<std::remove_cvref_t<const int&>, int>);
    auto greet = std::bind_front([](std::string_view a, std::string_view b) { return std::format("{} {}", a, b); }, "Hello");
    std::println("to_array size={} make_shared<int[]> ok={} bind_front: {}", arr.size(), shared_arr != nullptr, greet("world"));

    // <chrono> calendar: dates as types, with formatting support.
    using namespace std::chrono;
    year_month_day date = 2026y / November / 7d;
    year_month_day feb_end = 2026y / February / last;
    weekday wd{sys_days{date}};
    std::println("chrono calendar: {} is a {}, last day of Feb 2026: {}", date, wd, feb_end);
    std::println("days between: {}", (sys_days{date} - sys_days{2026y / January / 1d}).count());

    // std::jthread: joins automatically and supports cooperative cancellation
    // through std::stop_token.
    std::atomic<int> ticks{0};
    {
        std::jthread worker([&ticks](std::stop_token st) {
            while (!st.stop_requested()) {
                ++ticks;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        worker.request_stop();
    }   // joined here automatically
    std::println("jthread + stop_token: worker ran and was stopped cooperatively: {}", ticks.load() > 0);

    // std::latch (one-shot), std::barrier (reusable), std::counting_semaphore.
    constexpr int kThreads = 3;
    std::latch done(kThreads);
    std::barrier sync_point(kThreads);
    std::counting_semaphore<2> slots(2);
    std::atomic<int> phases{0};
    {
        std::vector<std::jthread> pool;
        for (int i = 0; i < kThreads; ++i) {
            pool.emplace_back([&] {
                slots.acquire();          // at most 2 threads inside at once
                phases.fetch_add(1);
                slots.release();
                sync_point.arrive_and_wait();
                done.count_down();
            });
        }
        done.wait();
    }
    std::println("latch/barrier/semaphore: {} phases completed", phases.load());

    // Atomic wait/notify (efficient futex-style blocking) and std::atomic_ref.
    std::atomic<int> flag{0};
    std::jthread notifier([&] { flag.store(1); flag.notify_one(); });
    flag.wait(0);                     // blocks while flag == 0
    int plain = 10;
    std::atomic_ref<int> ref(plain);  // atomic operations on a non-atomic object
    ref.fetch_add(5);
    std::println("atomic wait/notify flag={}, atomic_ref -> {}", flag.load(), plain);

    // std::osyncstream: synchronized output that never interleaves characters.
#if defined(__cpp_lib_syncbuf)
    std::osyncstream(std::cout) << "osyncstream: synchronized line\n";
#endif

    // std::assume_aligned tells the optimizer about pointer alignment.
    alignas(64) static float aligned_data[16] = {1.0f};
    float* ap = std::assume_aligned<64>(aligned_data);
    std::println("assume_aligned: first = {}", ap[0]);
}

} // namespace cpp20

// ############################################################################
//                                   C++23
// ############################################################################
namespace cpp23 {

// --- Deducing this (explicit object parameter) -------------------------------
// A member function can take the object as an explicit first parameter
// "this Self&& self". One function template then covers const/non-const and
// lvalue/rvalue overloads, and the derived type is known in the base: a
// simpler replacement for CRTP.
struct ShapeBase {
    void describe(this const auto& self) {
        std::println("deducing this (CRTP replacement): {} with area {:.2f}", self.name(), self.area());
    }
};
struct Square : ShapeBase {
    double side = 0;
    std::string_view name() const { return "square"; }
    double area() const { return side * side; }
};

class TextHolder {
public:
    explicit TextHolder(std::string s) : text_(std::move(s)) {}
    // One accessor for &, const&, && - returns std::string&, const std::string&,
    // or std::string&& accordingly. std::forward_like (C++23) forwards the
    // member with the value category of "self".
    template <typename Self>
    auto&& text(this Self&& self) { return std::forward_like<Self>(self.text_); }
private:
    std::string text_;
};

// --- Multidimensional subscript operator -------------------------------------------
// operator[] can now take several arguments: m[row, col].
template <typename T>
class Matrix {
public:
    Matrix(std::size_t rows, std::size_t cols) : rows_(rows), cols_(cols), data_(rows * cols) {}
    T& operator[](std::size_t r, std::size_t c) { return data_[r * cols_ + c]; }
    const T& operator[](std::size_t r, std::size_t c) const { return data_[r * cols_ + c]; }
    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }
private:
    std::size_t rows_, cols_;
    std::vector<T> data_;
};

// --- static operator() / static lambdas ----------------------------------------------
// A call operator that does not need "this" can be static, which removes the
// hidden object pointer (slightly faster, smaller code for function objects).
struct SquareFn {
    static constexpr int operator()(int x) { return x * x; }
};

// --- if consteval ------------------------------------------------------------------------
// Chooses a branch depending on whether we are in constant evaluation. Inside
// the "consteval" branch you may call consteval functions.
consteval int ct_only_twice(int x) { return 2 * x; }
constexpr int twice(int x) {
    if consteval {
        return ct_only_twice(x);       // compile-time path
    } else {
        return x + x;                  // run-time path (could use intrinsics, logging...)
    }
}
static_assert(twice(21) == 42);

// --- static constexpr variables inside constexpr functions -----------------------------
constexpr char hex_digit(int v) {
    static constexpr char table[] = "0123456789abcdef";   // allowed since C++23
    return table[v & 0xF];
}
static_assert(hex_digit(11) == 'b');

// --- [[assume(expr)]] ----------------------------------------------------------------------
// Portable optimizer hint: the expression is assumed true (never evaluated).
// If it is false the behavior is undefined, so use with care.
inline int divide_by_positive(int value, int divisor) {
    [[assume(divisor > 0)]];
    return value / divisor;
}

// --- std::expected: value OR error, without exceptions -------------------------------------
enum class ParseError { Empty, NotANumber, Negative };

std::string_view to_string(ParseError e) {
    switch (e) {
        case ParseError::Empty: return "empty input";
        case ParseError::NotANumber: return "not a number";
        case ParseError::Negative: return "negative";
    }
    // std::unreachable(): tells the compiler this point can never be reached.
    std::unreachable();
}

std::expected<int, ParseError> parse_non_negative(std::string_view s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    int value = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc{} || ptr != s.data() + s.size()) return std::unexpected(ParseError::NotANumber);
    if (value < 0) return std::unexpected(ParseError::Negative);
    return value;
}

// --- Simpler implicit move (P2266) -----------------------------------------------------------
// Returning a named rvalue reference now moves automatically. In C++20 this
// line failed to compile because "p" was treated as an lvalue.
inline std::unique_ptr<int> pass_through(std::unique_ptr<int>&& p) { return p; }

// --- std::out_ptr: adapt smart pointers to C APIs that return via T** ------------------------
inline void legacy_c_api_create(int** out) { *out = new int(2023); }

#if defined(__cpp_lib_generator)
// --- std::generator: the standard coroutine generator -----------------------------------------
std::generator<int> primes(int count) {
    for (int n = 2, found = 0; found < count; ++n) {
        bool is_prime = true;
        for (int d = 2; d * d <= n; ++d)
            if (n % d == 0) { is_prime = false; break; }
        if (is_prime) { ++found; co_yield n; }
    }
}
#endif

void run() {
    util::section("C++23");

    // std::print / std::println: type-safe, fast, Unicode-aware printing based
    // on std::format. (Used everywhere in this file.)
    std::print("std::print without newline, ");
    std::println("std::println with newline");

    // Deducing this
    Square sq; sq.side = 3;
    sq.describe();
    TextHolder holder("owned text");
    std::string moved_out = std::move(holder).text();     // picks the && version: moves
    std::println("deducing this + forward_like: moved '{}'", moved_out);

    // Recursive lambda via deducing this ("self" is the lambda itself).
    auto factorial = [](this auto self, int n) -> long long { return n <= 1 ? 1 : n * self(n - 1); };
    std::println("recursive lambda: 15! = {}", factorial(15));

    // Multidimensional subscript
    Matrix<int> mat(3, 3);
    for (std::size_t i = 0; i < mat.rows(); ++i)
        for (std::size_t j = 0; j < mat.cols(); ++j) mat[i, j] = static_cast<int>(i * 10 + j);
    std::println("multidimensional operator[]: mat[2, 1] = {}", mat[2, 1]);

    // static operator() and static lambdas
    auto triple = [](int x) static { return 3 * x; };
    std::println("static operator(): {} {}", SquareFn{}(9), triple(5));

    // Lambdas: "()" may be omitted even when using specifiers like mutable,
    // and attributes can be applied to lambdas.
    auto counter = [n = 0] mutable { return ++n; };
    counter();
    auto attributed = [] [[nodiscard]] (int x) { return x + 1; };
    std::println("lambda without (): {}, attributed lambda: {}", counter(), attributed(1));

    // auto(x): decay-copy in the language. Here it is needed because
    // std::erase takes the value by const&, and erasing the element that the
    // reference points to would be a bug.
    std::vector<int> v{7, 1, 7, 2, 7, 3};
    std::erase(v, auto(v.front()));
    std::println("auto(x) decay-copy: {}", util::join(v));

    // Literal suffix for size_t (uz) and ptrdiff_t (z).
    auto count = 10uz;
    static_assert(std::is_same_v<decltype(count), std::size_t>);
    std::println("size_t literal 10uz -> {}", count);

    // if consteval, static constexpr in constexpr fn, [[assume]]
    int runtime_value = static_cast<int>(v.size());
    std::println("if consteval runtime path: {}, hex_digit(14)={}, assume-divide={}",
                 twice(runtime_value), hex_digit(14), divide_by_positive(100, 7));

    // Named universal character escapes \N{...} and delimited escapes \u{...}, \x{...}.
    constexpr char32_t pi_named = U'\N{GREEK SMALL LETTER PI}';
    constexpr char32_t pi_delim = U'\u{3C0}';
    constexpr char hex_delim = '\x{41}';
    static_assert(pi_named == pi_delim && hex_delim == 'A');
    std::println("\\N{{...}} and \\u{{...}} escapes: U+{:04X}, '{}'", static_cast<std::uint32_t>(pi_named), hex_delim);

    // std::expected with monadic operations: and_then / transform / or_else.
    for (std::string_view input : {"42", "", "abc", "-5"}) {
        auto result = parse_non_negative(input)
                          .transform([](int x) { return x * 2; })
                          .and_then([](int x) -> std::expected<int, ParseError> {
                              if (x > 1000) return std::unexpected(ParseError::NotANumber);
                              return x;
                          });
        if (result) std::println("expected: '{}' -> {}", input, *result);
        else std::println("expected: '{}' -> error: {}", input, to_string(result.error()));
    }

    // std::optional monadic operations (and_then, transform, or_else).
    std::optional<std::string> maybe_name = "ada";
    auto upper_len = maybe_name
        .transform([](std::string s) { for (auto& ch : s) ch = static_cast<char>(std::toupper(ch)); return s; })
        .and_then([](const std::string& s) -> std::optional<std::size_t> { return s.size(); })
        .or_else([] { return std::optional<std::size_t>{0}; });
    std::println("optional monadic: length = {}", *upper_len);

    // Utilities: std::to_underlying, std::byteswap, std::string::contains,
    // resize_and_overwrite, std::invoke_r, std::bind_back.
    enum class Level : std::uint8_t { Low = 1, High = 9 };
    std::println("to_underlying={} byteswap(0x12345678)={:#x}", std::to_underlying(Level::High),
                 std::byteswap(0x12345678u));
    std::string text = "C++23 is here";
    std::println("string::contains: {}", text.contains("23"));
    std::string buffer;
    buffer.resize_and_overwrite(16, [](char* p, std::size_t n) {
        // Write directly into the string's storage without zero-initializing it first.
        auto res = std::to_chars(p, p + n, 123456789);
        return static_cast<std::size_t>(res.ptr - p);
    });
    std::println("resize_and_overwrite: '{}'", buffer);
    std::println("invoke_r<double>: {}", std::invoke_r<double>([] { return 7; }));
    auto minus_one = std::bind_back(std::minus<>{}, 1);
    std::println("bind_back: {}", minus_one(10));

    // Simpler implicit move
    auto moved = pass_through(std::make_unique<int>(5));
    std::println("implicit move from T&&: {}", *moved);

    // std::out_ptr
#if defined(__cpp_lib_out_ptr)
    std::unique_ptr<int> from_c;
    legacy_c_api_create(std::out_ptr(from_c));
    std::println("std::out_ptr: {}", *from_c);
#endif

    // std::ranges::to: materialize a view into a container.
    auto squares = std::views::iota(1, 6)
                 | std::views::transform([](int x) { return x * x; })
                 | std::ranges::to<std::vector>();
    std::println("ranges::to<vector>: {}", util::join(squares));

    // New C++23 views and algorithms (each guarded by its feature-test macro).
    std::vector<int> data{1, 2, 3, 4, 5, 6, 7};
    std::vector<std::string> names{"ann", "bob", "cid"};

#if defined(__cpp_lib_ranges_zip)
    // views::zip: iterate several ranges in lockstep (tuples of elements).
    for (auto [n, name] : std::views::zip(data, names)) std::println("views::zip: {} {}", n, name);
    // views::adjacent<N> / pairwise_transform: sliding tuples of neighbors.
    for (auto [a, b] : data | std::views::pairwise) std::print("({},{}) ", a, b);
    std::println("<- views::pairwise");
    auto diffs = data | std::views::pairwise_transform([](int a, int b) { return b - a; });
    std::println("views::pairwise_transform diffs: {}", util::join(diffs));
#endif
#if defined(__cpp_lib_ranges_enumerate)
    // views::enumerate: (index, element) pairs - no manual counters.
    for (auto [i, name] : names | std::views::enumerate) std::println("views::enumerate: {} -> {}", i, name);
#endif
#if defined(__cpp_lib_ranges_chunk)
    // views::chunk: non-overlapping groups of N.
    for (auto chunk : data | std::views::chunk(3)) std::print("{} ", util::join(chunk));
    std::println("<- views::chunk(3)");
#endif
#if defined(__cpp_lib_ranges_slide)
    // views::slide: overlapping windows of N.
    for (auto window : data | std::views::slide(3)) std::print("{} ", util::join(window));
    std::println("<- views::slide(3)");
#endif
#if defined(__cpp_lib_ranges_chunk_by)
    // views::chunk_by: split where a binary predicate between neighbors fails.
    std::vector<int> runs{1, 2, 3, 1, 2, 5, 6, 0};
    for (auto run : runs | std::views::chunk_by(std::ranges::less{})) std::print("{} ", util::join(run));
    std::println("<- views::chunk_by(less)");
#endif
#if defined(__cpp_lib_ranges_stride)
    // views::stride: every N-th element.
    std::println("views::stride(2): {}", util::join(data | std::views::stride(2)));
#endif
#if defined(__cpp_lib_ranges_cartesian_product)
    // views::cartesian_product: all combinations (nested loops as a view).
    std::string_view suits = "SH";
    for (auto [rank, suit] : std::views::cartesian_product(std::views::iota(1, 3), suits))
        std::print("{}{} ", rank, suit);
    std::println("<- views::cartesian_product");
#endif
#if defined(__cpp_lib_ranges_join_with)
    // views::join_with: flatten with a separator between the inner ranges.
    std::string joined = names | std::views::join_with(std::string_view(" | ")) | std::ranges::to<std::string>();
    std::println("views::join_with: {}", joined);
#endif
#if defined(__cpp_lib_ranges_repeat)
    // views::repeat: an infinite (or bounded) repetition of a value.
    std::println("views::repeat: {}", util::join(std::views::repeat(7, 4)));
#endif
#if defined(__cpp_lib_ranges_as_rvalue)
    // views::as_rvalue: elements become rvalues, so copying moves them.
    std::vector<std::string> source{"move", "me"};
    auto stolen = source | std::views::as_rvalue | std::ranges::to<std::vector>();
    std::println("views::as_rvalue: moved {} strings", stolen.size());
#endif
#if defined(__cpp_lib_ranges_as_const)
    // views::as_const: read-only view of the elements.
    for ([[maybe_unused]] const int& x : data | std::views::as_const) {}
    std::println("views::as_const: ok");
#endif
#if defined(__cpp_lib_ranges_fold)
    // ranges::fold_left / fold_right: proper folds (accumulate for ranges).
    std::println("ranges::fold_left sum = {}, fold_right string = {}",
                 std::ranges::fold_left(data, 0, std::plus<>{}),
                 std::ranges::fold_right(names, std::string{}, [](const std::string& s, std::string acc) { return s + acc; }));
#endif
#if defined(__cpp_lib_ranges_contains)
    std::println("ranges::contains(5) = {}, contains_subrange = {}", std::ranges::contains(data, 5),
                 std::ranges::contains_subrange(data, std::array{3, 4}));
#endif
#if defined(__cpp_lib_ranges_starts_ends_with)
    std::println("ranges::starts_with = {}", std::ranges::starts_with(data, std::array{1, 2}));
#endif
#if defined(__cpp_lib_ranges_find_last)
    std::vector<int> repeated{1, 2, 1, 3};
    auto last_one = std::ranges::find_last(repeated, 1);
    std::println("ranges::find_last: index {}", std::distance(repeated.begin(), last_one.begin()));
#endif
#if defined(__cpp_lib_ranges_iota)
    std::vector<int> seq(5);
    std::ranges::iota(seq, 100);
    std::println("ranges::iota: {}", util::join(seq));
#endif

    // Formatting ranges, tuples and debug strings directly ({:?} escapes).
#if defined(__cpp_lib_format_ranges)
    std::vector<std::pair<int, std::string>> table{{1, "one"}, {2, "two"}};
    std::println("format ranges: {} {} {:?}", data, table, std::string("tab\there"));
#endif

    // std::mdspan: a multidimensional, non-owning view over contiguous data.
#if defined(__cpp_lib_mdspan)
    std::vector<double> grid(12);
    std::iota(grid.begin(), grid.end(), 0.0);
    std::mdspan view(grid.data(), 3, 4);              // 3 rows x 4 cols (CTAD)
    double diagonal = 0;
    for (std::size_t i = 0; i < view.extent(0); ++i) diagonal += view[i, i];
    std::println("mdspan: rank={} extents={}x{} view[2, 3]={} diagonal sum={}",
                 view.rank(), view.extent(0), view.extent(1), view[2, 3], diagonal);
#endif

    // std::flat_map / std::flat_set: sorted associative containers stored in
    // contiguous vectors - cache friendly, faster lookups for small/medium data.
#if defined(__cpp_lib_flat_map)
    std::flat_map<std::string, int> fm;
    fm["zeta"] = 26; fm["alpha"] = 1; fm["mu"] = 12;
    std::string fm_text;
    for (const auto& [k, val] : fm) fm_text += std::format("{}={} ", k, val);
    std::println("flat_map (sorted, contiguous): {}", fm_text);
#endif
#if defined(__cpp_lib_flat_set)
    std::flat_set<int> fs{5, 1, 3, 1, 5};
    std::println("flat_set: {}", util::join(fs));
#endif

    // std::generator
#if defined(__cpp_lib_generator)
    std::println("std::generator primes: {}", util::join(primes(8)));
#endif

    // std::move_only_function: like std::function but supports move-only callables.
#if defined(__cpp_lib_move_only_function)
    std::move_only_function<int()> task = [p = std::make_unique<int>(77)] { return *p; };
    std::println("move_only_function: {}", task());
#endif

    // std::spanstream: iostreams over a fixed buffer, no allocations.
#if defined(__cpp_lib_spanstream)
    char storage[64]{};
    std::ospanstream os{std::span<char>(storage)};
    os << "spanstream " << 42;
    std::println("{}", std::string_view(os.span().data(), os.span().size()));
#endif

    // std::stacktrace: capture the current call stack at run time.
#if defined(__cpp_lib_stacktrace)
    auto trace = std::stacktrace::current();
    std::println("std::stacktrace: captured {} frame(s)", trace.size());
#endif

    // Preprocessor: #elifdef / #elifndef and #warning.
#ifdef SHOWCASE_SOME_UNDEFINED_MACRO
#  warning "This #warning would print a diagnostic but keep compiling"
#elifdef __cplusplus
    std::println("#elifdef: __cplusplus = {}", __cplusplus);
#endif

    // Labels at the end of a compound statement (C compatibility).
    {
        goto end_of_block;
    end_of_block:
    }
    std::println("label at end of block: ok");
}

} // namespace cpp23

int main() {
    std::println("Modern C++ feature showcase (C++11 .. C++23), __cplusplus = {}", __cplusplus);
    cpp11::run();
    cpp14::run();
    cpp17::run();
    cpp20::run();
    cpp23::run();
    std::println("");
    std::println("Done.");
}
