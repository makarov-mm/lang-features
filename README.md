# Modern Language Features Showcase

Four self-contained source files that walk through **~15 years of language evolution** in
**C++**, **Rust**, **C#** and **Python**. Each file compiles/runs on its own, prints what every
feature does, and documents each feature in English comments: *which version introduced it,
how it works, and what it is used for*.

No projects, no build scripts, no third-party dependencies — one file per language.

| File | Language range | Lines | Toolchain |
|---|---|---:|---|
| [`modern_cpp_showcase.cpp`](modern_cpp_showcase.cpp) | C++11 → **C++23** | ~1700 | Visual Studio 2026 (MSVC 14.51+), GCC 14+, Clang 20+ |
| [`modern_rust_showcase.rs`](modern_rust_showcase.rs) | Rust 1.0 → **1.97** (editions 2015 → 2024) | ~1200 | rustc 1.97+ |
| [`ModernCSharpShowcase.cs`](ModernCSharpShowcase.cs) | C# 5 → **C# 14** (.NET 10) | ~1250 | .NET 10 SDK / Visual Studio 2026 |
| [`modern_python_showcase.py`](modern_python_showcase.py) | Python 3.2 → **3.15** | ~950 | CPython 3.15+ |

## Quick start

### C++ (C++23)

```bat
:: Developer Command Prompt for Visual Studio 2026
cl /std:c++latest /EHsc /W4 /O2 /utf-8 /Zc:__cplusplus modern_cpp_showcase.cpp
modern_cpp_showcase.exe
```

`/std:c++23preview` works too. In the IDE: create an empty C++ project, add the file and set
*C/C++ → Language → C++ Language Standard* to `/std:c++latest`.

Other compilers:

```sh
g++ -std=c++23 -O2 modern_cpp_showcase.cpp -lstdc++exp
clang++ -std=c++23 -stdlib=libc++ -O2 modern_cpp_showcase.cpp
```

### Rust

```sh
rustc --edition 2024 -O modern_rust_showcase.rs
./modern_rust_showcase
```

### C#

```sh
dotnet run ModernCSharpShowcase.cs
```

The file is a .NET 10 *file-based app*: the `#:property` directives at the top replace a
`.csproj` (language version, nullable, unsafe code). On Unix it can also be run directly
thanks to the `#!` shebang: `chmod +x ModernCSharpShowcase.cs && ./ModernCSharpShowcase.cs`.

### Python

```sh
python3.15 modern_python_showcase.py      # Windows: py -3.15 modern_python_showcase.py
```

## What is covered

<details>
<summary><b>C++11 → C++23</b></summary>

- **C++11** — `auto`/`decltype`, lambdas, move semantics & perfect forwarding, variadic templates,
  `enum class`, `constexpr`, `static_assert`, user-defined & raw string literals, smart pointers,
  `override`/`final`, delegating/inheriting constructors, `std::thread`/`mutex`/`atomic`/`async`,
  `thread_local`, `<chrono>`, `<random>`, `<regex>`
- **C++14** — generic lambdas, init-captures, return type deduction, `decltype(auto)`, relaxed
  `constexpr`, variable templates, binary literals & digit separators, `[[deprecated]]`,
  `std::make_unique`, transparent comparators
- **C++17** — structured bindings, `if constexpr`, fold expressions, CTAD, `if`/`switch` with
  initializer, inline variables, nested namespaces, `optional`/`variant`/`any`/`string_view`,
  `std::filesystem`, `<charconv>`, parallel algorithms, `std::pmr`, guaranteed copy elision
- **C++20** — concepts & `requires`, ranges, coroutines (hand-written generator and task),
  `<=>`, designated initializers, `consteval`/`constinit`, constexpr `vector`/`string`/virtual,
  class-type NTTPs, `std::format`, `std::span`, `<bit>`, `<numbers>`, calendar `<chrono>`,
  `jthread`/`stop_token`, `latch`/`barrier`/`semaphore`, `source_location`
- **C++23** — deducing `this`, `if consteval`, multidimensional `operator[]`, `static operator()`,
  `auto(x)`, `[[assume]]`, `\N{...}` escapes, `uz` literals, `std::print`, `std::expected`,
  monadic `optional`, `std::mdspan`, `std::flat_map`/`flat_set`, `std::generator`,
  `std::stacktrace`, `ranges::to`, `views::zip`/`enumerate`/`chunk`/`slide`/`cartesian_product`/…,
  `ranges::fold_left`, `#elifdef`/`#warning`

Library features that some standard libraries still lack are wrapped in C++20 feature-test
macros (`#if defined(__cpp_lib_flat_map)` …). With MSVC and `/std:c++latest` all of them are active.

</details>

<details>
<summary><b>Rust 1.0 → 1.97</b></summary>

- **1.0 – 1.30** — ownership & borrowing, traits, enums & `match`, `?` operator, `impl Trait`,
  `dyn Trait`, `main` returning `Result`, `u128`, `..=`, match ergonomics, unions, raw identifiers
- **Edition 2018** — NLL, `const fn`, `dbg!`, `async`/`await` (with a tiny std-only executor),
  `#[non_exhaustive]`, `matches!`, subslice patterns, `#[track_caller]`, const generics, `ControlFlow`
- **Edition 2021** — disjoint closure captures, format-string captures, destructuring assignment,
  scoped threads, GATs, `let … else`, labeled block `break`, `OnceLock`/`LazyLock`,
  `async fn` in traits, C-string literals `c"…"`, `offset_of!`, inline `const {}`, `&raw`,
  `unsafe extern` + `safe` items, `#[unsafe(no_mangle)]`, `use<..>` precise capturing, `#[expect]`
- **Edition 2024 (1.85 – 1.97)** — async closures & `AsyncFn`, trait upcasting,
  `get_disjoint_mut`, `extract_if`, `io::pipe`, let chains, `File::lock`, `[T; _]`,
  `fmt::from_fn`, `array_windows`, if-let guards, `cfg_select!`, `assert_matches!`,
  `core::range` types, atomic `update`, `bit_width`/`highest_one`

</details>

<details>
<summary><b>C# 5 → C# 14</b></summary>

- **C# 5 / 6** — `async`/`await`, caller info, string interpolation, `nameof`, `?.`, expression-bodied
  members, auto-property initializers, `using static`, exception filters
- **C# 7.x** — tuples & deconstruction, pattern matching, local functions, `out var`, ref
  locals/returns, `ValueTask`, `in`, `readonly struct`, `ref struct`, `private protected`
- **C# 8** — nullable reference types, async streams, indices & ranges, switch expressions,
  recursive patterns, `using` declarations, default interface methods, `??=`
- **C# 9** — records, `init`, top-level statements, `and`/`or`/`not` and relational patterns,
  `nint`, function pointers, module initializers, covariant returns
- **C# 10** — record structs, `global using`, extended property patterns, lambda natural types,
  `CallerArgumentExpression`, interpolated string handlers
- **C# 11** — raw string literals, generic math (static abstract interface members), list
  patterns, `required`, `file` types, UTF-8 literals, `ref` fields & `scoped`, `>>>`
- **C# 12** — primary constructors, collection expressions, inline arrays, `ref readonly`
  parameters, alias any type, `[Experimental]`
- **C# 13** — `params` collections, `System.Threading.Lock`, `\e`, partial properties,
  `allows ref struct`, `OverloadResolutionPriority`
- **C# 14** — extension members (properties, static members, operators), the `field` keyword,
  null-conditional assignment, `nameof(List<>)`, implicit span conversions, partial constructors
  & events, user-defined compound assignment operators
- **.NET 5 – 10 library** — source-generated JSON and Regex, `DateOnly`, LINQ
  `Chunk`/`CountBy`/`AggregateBy`/`Index`/`LeftJoin`/`Shuffle`, `FrozenDictionary`,
  `SearchValues`, `TimeProvider`, `Task.WhenEach`, Channels, `Guid.CreateVersion7`, `Base64Url`

</details>

<details>
<summary><b>Python 3.2 → 3.15</b></summary>

- **3.2 – 3.5** — `concurrent.futures`, `lru_cache`, `yield from`, `enum`, `pathlib`,
  `statistics`, `singledispatch`, `async`/`await`, `@` operator, PEP 448 unpacking
- **3.6 – 3.7** — f-strings, numeric underscores, variable annotations, async generators,
  `__init_subclass__`, `__set_name__`, `dataclasses`, `contextvars`, module `__getattr__`
- **3.8 – 3.9** — walrus `:=`, positional-only parameters, `f"{x=}"`, `Protocol`, `TypedDict`,
  `Final`, `Literal`, `cached_property`, dict `|`, `removeprefix`, `zoneinfo`, `graphlib`
- **3.10 – 3.11** — structural pattern matching, `X | Y` unions, `ParamSpec`, exception groups &
  `except*`, `TaskGroup`, `asyncio.timeout`, `tomllib`, `Self`, `StrEnum`, `add_note`
- **3.12 – 3.13** — PEP 695 generics & `type` aliases, PEP 701 f-strings, `@override`,
  `itertools.batched`, type parameter defaults, `@deprecated`, `TypeIs`, `ReadOnly`, `copy.replace`
- **3.14** — template strings `t"…"`, deferred annotations & `annotationlib`, `except A, B:`,
  `concurrent.interpreters`, `compression.zstd`, `uuid7`, `Path.copy`/`move`
- **3.15** — `lazy import`, `frozendict`, `sentinel`, unpacking in comprehensions `[*x for x in …]`,
  UTF-8 mode by default, closed `TypedDict`, `TypeForm`, `@disjoint_base`, `math.integer`

</details>

## Tested with

| Language | Verified on |
|---|---|
| C++ | GCC 14.2 (libstdc++) and Clang 20.1 (libc++), `-Wall -Wextra -pedantic`, no warnings |
| Rust | rustc 1.97.0, edition 2024, no warnings |
| C# | .NET SDK 10.0.114, no warnings |
| Python | CPython 3.15.2, `-X dev`, no warnings |

The C++ file targets MSVC 14.51 (Visual Studio 2026 v18.6+), which implements nearly all of
C++23. The two features MSVC still lacks (P2564 consteval propagation, P0533 constexpr
`<cmath>`) are intentionally not used.

All files are pure ASCII, so there are no source-encoding issues on any platform.
