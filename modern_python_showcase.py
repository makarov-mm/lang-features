"""
===============================================================================
 modern_python_showcase.py

 A single-file tour of Python 3 features from Python 3.2 (2011) up to
 Python 3.15 (October 2026). Every feature is commented with the version that
 introduced it, how it works and what it is used for.

 Run:
     python3.15 modern_python_showcase.py      (Windows: py -3.15 modern_python_showcase.py)

 Requires Python 3.15+: the file uses 3.15 syntax (lazy imports, unpacking in
 comprehensions) and 3.14 syntax (template strings, except without
 parentheses), so older interpreters stop with a SyntaxError right away.
 Only the standard library is used.
===============================================================================
"""

# --- Python 3.15: explicit lazy imports (PEP 810) ---------------------------
# "lazy import X" binds a lightweight proxy; the module is really imported on
# first attribute access. This cuts start-up time of CLIs and big apps that
# import many modules but use few of them on a given run. Lazy imports are
# only allowed at module level (not inside functions, classes or try blocks).
lazy import difflib
lazy from fractions import Fraction

import sys

# Captured before anything touches the lazy names:
DIFFLIB_LOADED_AT_START = "difflib" in sys.modules

import annotationlib
import asyncio
import base64
import contextlib
import contextvars
import copy
import dataclasses
import datetime as dt
import enum
import functools
import graphlib
import heapq
import ipaddress
import itertools
import json
import math
import math.integer
import operator
import os
import pathlib
import secrets
import statistics
import string.templatelib
import tempfile
import tomllib
import types
import typing
import uuid
import warnings
from collections.abc import AsyncIterator, Callable
from compression import zstd
from concurrent import interpreters
from concurrent.futures import InterpreterPoolExecutor, ThreadPoolExecutor
from typing import (
    Annotated, Any, Concatenate, Final, Literal, LiteralString, NamedTuple,
    NotRequired, ParamSpec, Protocol, ReadOnly, Required, Self, TypedDict, TypeForm,
    TypeGuard, TypeIs, assert_never, disjoint_base, override, runtime_checkable,
)


def section(title: str) -> None:
    print(f"\n=== {title} ===")


# #############################################################################
#  Python 3.2 - 3.5 (2011 - 2015)
# #############################################################################
def demo_3_2_to_3_5() -> None:
    section("Python 3.2 - 3.5")
    from concurrent.futures import as_completed

    # 3.2: concurrent.futures - thread/process pools with futures.
    with ThreadPoolExecutor(max_workers=4) as pool:
        futures = [pool.submit(pow, 2, n) for n in range(5)]
        results = sorted(f.result() for f in as_completed(futures))
    print(f"concurrent.futures: {results}")

    # 3.2: functools.lru_cache - memoization decorator with a bounded cache.
    @functools.lru_cache(maxsize=None)
    def fib(n: int) -> int:
        return n if n < 2 else fib(n - 1) + fib(n - 2)
    print(f"lru_cache: fib(80) = {fib(80)}, cache hits = {fib.cache_info().hits}")

    # 3.3: "yield from" delegates to a sub-generator.
    def flatten(nested):
        for item in nested:
            if isinstance(item, list):
                yield from flatten(item)
            else:
                yield item
    print(f"yield from: {list(flatten([1, [2, [3, 4]], 5]))}")

    # 3.3: "raise ... from None" suppresses the chained context in tracebacks.
    try:
        try:
            {}["missing"]
        except KeyError:
            raise LookupError("clean error message") from None
    except LookupError as e:
        print(f"raise from None: {e!r}, __suppress_context__={e.__suppress_context__}")

    # 3.3: __qualname__ and the ipaddress module.
    net = ipaddress.ip_network("192.168.0.0/30")
    print(f"__qualname__: {fib.__wrapped__.__qualname__}; ipaddress hosts: {[str(h) for h in net.hosts()]}")

    # 3.4: enum, pathlib, statistics, singledispatch, contextlib.suppress.
    class Color(enum.Enum):
        RED = 1
        GREEN = 2
    p = pathlib.PurePosixPath("/srv/app/config/settings.toml")
    print(f"enum: {Color.GREEN!r}; pathlib: name={p.name} suffix={p.suffix} parent={p.parent}")
    print(f"statistics: mean={statistics.mean([1, 2, 3, 4])} median={statistics.median([3, 1, 2])} "
          f"stdev={statistics.stdev([2, 4, 4, 4, 5, 5, 7, 9]):.3f}")

    @functools.singledispatch
    def describe(x: object) -> str:
        return "something"

    @describe.register
    def _(x: int) -> str:
        return f"int {x}"

    @describe.register
    def _(x: list) -> str:
        return f"list of {len(x)}"
    print(f"singledispatch: {describe(5)}, {describe([1, 2])}, {describe(1.5)}")

    with contextlib.suppress(FileNotFoundError):
        os.remove("/definitely/not/here")
    print("contextlib.suppress: ignored FileNotFoundError")

    # 3.5: async/await syntax (asyncio arrived in 3.4 as a library).
    async def fetch(n: int) -> int:
        await asyncio.sleep(0.001)
        return n * 10
    async def main() -> list[int]:
        return await asyncio.gather(*(fetch(i) for i in range(3)))
    print(f"async/await: {asyncio.run(main())}")

    # 3.5: matrix multiplication operator "@" (__matmul__).
    class Mat:
        def __init__(self, rows): self.rows = rows
        def __matmul__(self, other):
            cols = list(zip(*other.rows))
            return Mat([[sum(a * b for a, b in zip(r, c)) for c in cols] for r in self.rows])
    print(f"@ operator: {(Mat([[1, 2], [3, 4]]) @ Mat([[5, 6], [7, 8]])).rows}")

    # 3.5: additional unpacking generalizations (PEP 448).
    a, b = [1, 2], [3]
    d1, d2 = {"x": 1}, {"y": 2}
    print(f"PEP 448 unpacking: {[*a, *b, 4]}, {{**d1, **d2}} = {({**d1, **d2})}, max(*a, *b) = {max(*a, *b)}")

    # 3.5: type hints (typing module), os.scandir, math.isclose.
    print(f"math.isclose(0.1 + 0.2, 0.3) = {math.isclose(0.1 + 0.2, 0.3)}")


# #############################################################################
#  Python 3.6 - 3.7 (2016 - 2018)
# #############################################################################
class Plugin:
    # 3.6: __init_subclass__ - hook called when a class is subclassed.
    # Replaces many metaclass use cases (e.g. plugin registries).
    registry: dict[str, type] = {}

    def __init_subclass__(cls, /, name: str = "", **kwargs: Any) -> None:
        super().__init_subclass__(**kwargs)
        Plugin.registry[name or cls.__name__] = cls


class CsvPlugin(Plugin, name="csv"):
    pass


class Positive:
    # 3.6: __set_name__ - descriptors learn the attribute name they are bound to.
    def __set_name__(self, owner: type, name: str) -> None:
        self.private = "_" + name

    def __get__(self, obj: Any, objtype: Any = None) -> Any:
        return getattr(obj, self.private)

    def __set__(self, obj: Any, value: float) -> None:
        if value <= 0:
            raise ValueError("must be positive")
        setattr(obj, self.private, value)


class Account:
    balance = Positive()

    def __init__(self, balance: float) -> None:
        self.balance = balance


# 3.6: class-based NamedTuple syntax with annotations and defaults.
class Point(NamedTuple):
    x: float
    y: float = 0.0


# 3.7: dataclasses - generate __init__, __repr__, __eq__ (and more) from
# annotated fields.
@dataclasses.dataclass(order=True)
class Version:
    major: int
    minor: int = 0
    tags: list[str] = dataclasses.field(default_factory=list, compare=False)

    def __post_init__(self) -> None:
        if self.major < 0:
            raise ValueError("negative version")


request_id: contextvars.ContextVar[str] = contextvars.ContextVar("request_id", default="none")


def demo_3_6_to_3_7() -> None:
    section("Python 3.6 - 3.7")

    # 3.6: f-strings, with expressions and format specs.
    name, value = "pi", math.pi
    print(f"f-strings: {name} = {value:.4f}, {name.upper()!r}, {255:#x}, {1234567:,}")

    # 3.6: underscores in numeric literals.
    budget = 1_000_000.50
    print(f"numeric underscores: {budget:,.2f}, {0x_FF_FF}")

    # 3.6: variable annotations (PEP 526).
    counter: int = 0
    counter += 1
    print(f"variable annotations: counter={counter}")

    # 3.6: secrets - cryptographically strong random values.
    print(f"secrets: token_hex(8) has {len(secrets.token_hex(8))} chars")

    # 3.6: async generators and async comprehensions.
    async def ticker(n: int) -> AsyncIterator[int]:
        for i in range(n):
            await asyncio.sleep(0)
            yield i
    async def collect() -> list[int]:
        return [i * i async for i in ticker(5) if i % 2 == 0]
    print(f"async generator + async comprehension: {asyncio.run(collect())}")

    print(f"__init_subclass__ registry: {list(Plugin.registry)}")
    acc = Account(10)
    try:
        acc.balance = -5
    except ValueError as e:
        print(f"__set_name__ descriptor: balance={acc.balance}, rejected: {e}")
    print(f"NamedTuple class syntax: {Point(1.5)}, enum.auto/Flag below")

    class Perm(enum.Flag):
        READ = enum.auto()
        WRITE = enum.auto()
        EXEC = enum.auto()
    print(f"enum.Flag: {Perm.READ | Perm.WRITE!r}, WRITE in rw: {Perm.WRITE in (Perm.READ | Perm.WRITE)}")

    # 3.6 (CPython) / 3.7 (language): dicts preserve insertion order.
    ordered = {"z": 1, "a": 2, "m": 3}
    print(f"dict insertion order guaranteed: {list(ordered)}")

    # 3.7: dataclasses
    v1, v2 = Version(1, 2, ["beta"]), Version(1, 10)
    print(f"dataclass: {v1}, v1 < v2: {v1 < v2}, astuple: {dataclasses.astuple(v2)}")

    # 3.7: contextvars - context-local state that works with asyncio tasks.
    async def handle(rid: str) -> str:
        request_id.set(rid)
        await asyncio.sleep(0)
        return request_id.get()
    async def serve() -> list[str]:
        return await asyncio.gather(handle("A"), handle("B"))
    print(f"contextvars per task: {asyncio.run(serve())}, outside: {request_id.get()}")

    # 3.7: module-level __getattr__ (PEP 562) - lazy or deprecated attributes.
    mod = types.ModuleType("dynamic_mod")
    mod.__getattr__ = lambda attr: f"<computed {attr}>"
    print(f"module __getattr__: {mod.anything}")

    # 3.7: time.time_ns(), breakpoint() (drops into pdb; not called here),
    # asyncio.run(), and __class_getitem__ for generic classes.
    import time
    print(f"time_ns is int: {isinstance(time.time_ns(), int)}")


# #############################################################################
#  Python 3.8 - 3.9 (2019 - 2020)
# #############################################################################
@runtime_checkable
class SupportsClose(Protocol):
    # 3.8: Protocol - structural typing ("static duck typing"). Any class with
    # a matching close() method satisfies it, without inheriting from it.
    def close(self) -> None: ...


class Resource:
    def close(self) -> None:
        pass


class MovieInfo(TypedDict):
    # 3.8: TypedDict - typed dictionaries with fixed keys.
    title: str
    year: int


MAX_RETRIES: Final = 3          # 3.8: Final - must not be reassigned (checked by type checkers)
Mode = Literal["r", "w"]        # 3.8: Literal - only these exact values


def open_mode(mode: Mode) -> str:
    return {"r": "read", "w": "write"}[mode]


def demo_3_8_to_3_9() -> None:
    section("Python 3.8 - 3.9")

    # 3.8: assignment expressions ("walrus") :=
    data = [3, 8, 1, 9, 4]
    if (n := len(data)) > 3:
        print(f"walrus: list has {n} items")
    chunks = []
    stream = iter(data)
    while (chunk := list(itertools.islice(stream, 2))):
        chunks.append(chunk)
    print(f"walrus in while: {chunks}")

    # 3.8: positional-only parameters "/" (and keyword-only "*" since 3.0).
    def clamp(value, /, low=0, high=10, *, strict=False):
        if strict and not low <= value <= high:
            raise ValueError(value)
        return max(low, min(high, value))
    print(f"positional-only params: {clamp(42)}, {clamp(-3, low=-1)}")

    # 3.8: self-documenting f-string "=".
    width, height = 3, 4
    print(f"f-string '=': {width=}, {height * 2 = }, {math.hypot(width, height)=:.1f}")

    print(f"Protocol: isinstance(Resource(), SupportsClose) = {isinstance(Resource(), SupportsClose)}")
    movie: MovieInfo = {"title": "Metropolis", "year": 1927}
    print(f"TypedDict: {movie}, Final: {MAX_RETRIES}, Literal: {open_mode('w')}")

    # 3.8: functools.cached_property, math.prod/isqrt/dist/comb/perm.
    class Circle:
        def __init__(self, r: float) -> None:
            self.r = r

        @functools.cached_property
        def area(self) -> float:
            print("  (computing area once)")
            return math.pi * self.r ** 2
    c = Circle(2)
    print(f"cached_property: {c.area:.3f} / {c.area:.3f}")
    print(f"math: prod={math.prod([1, 2, 3, 4])} isqrt(99)={math.isqrt(99)} "
          f"dist={math.dist((0, 0), (3, 4))} comb(5,2)={math.comb(5, 2)} perm(5,2)={math.perm(5, 2)}")
    print(f"reversed(dict): {list(reversed({'a': 1, 'b': 2}))}")

    # 3.9: dict merge | and update |= operators.
    defaults = {"color": "blue", "size": "M"}
    user = {"size": "L"}
    merged = defaults | user
    merged |= {"extra": True}
    print(f"dict |: {merged}")

    # 3.9: str.removeprefix / removesuffix.
    print(f"removeprefix/suffix: {'test_case.py'.removeprefix('test_').removesuffix('.py')}")

    # 3.9: built-in generics in annotations: list[int], dict[str, int] ...
    def totals(m: dict[str, list[int]]) -> dict[str, int]:
        return {k: sum(v) for k, v in m.items()}
    print(f"builtin generics: {totals({'a': [1, 2], 'b': [3]})}")

    # 3.9: zoneinfo - IANA time zones in the standard library.
    try:
        from zoneinfo import ZoneInfo
        berlin = dt.datetime(2026, 7, 1, 12, tzinfo=ZoneInfo("Europe/Berlin"))
        print(f"zoneinfo: {berlin.isoformat()} -> UTC {berlin.astimezone(dt.UTC).time()}")
    except Exception as e:  # on Windows the 'tzdata' package may be needed
        print(f"zoneinfo: time zone data unavailable ({type(e).__name__})")

    # 3.9: graphlib.TopologicalSorter - dependency ordering.
    deps = {"app": {"lib", "config"}, "lib": {"core"}, "config": {"core"}}
    print(f"graphlib topological order: {list(graphlib.TopologicalSorter(deps).static_order())}")

    # 3.9: functools.cache, math.lcm, Annotated, relaxed decorator grammar.
    @functools.cache
    def slow_square(x: int) -> int:
        return x * x
    Meters = Annotated[float, "unit: meters"]
    distance: Meters = 5.0
    decorators = {"twice": lambda f: lambda x: 2 * f(x)}

    @decorators["twice"]          # 3.9: any expression is allowed as a decorator
    def inc(x: int) -> int:
        return x + 1
    print(f"cache: {slow_square(12)}, lcm(4, 6)={math.lcm(4, 6)}, Annotated metadata: {Meters.__metadata__}, "
          f"expression decorator: {inc(3)}, distance={distance}")


# #############################################################################
#  Python 3.10 - 3.11 (2021 - 2022)
# #############################################################################
P = ParamSpec("P")


def logged(func: Callable[P, int]) -> Callable[P, int]:
    # 3.10: ParamSpec - decorators that preserve the wrapped function's signature.
    @functools.wraps(func)
    def wrapper(*args: P.args, **kwargs: P.kwargs) -> int:
        result = func(*args, **kwargs)
        print(f"  logged {func.__name__}{args} -> {result}")
        return result
    return wrapper


@logged
def add(a: int, b: int) -> int:
    return a + b


def with_retries(func: Callable[Concatenate[int, P], str]) -> Callable[P, str]:
    # 3.10: Concatenate - a decorator that supplies the first argument.
    def inner(*args: P.args, **kwargs: P.kwargs) -> str:
        return func(3, *args, **kwargs)
    return inner


@with_retries
def download(retries: int, url: str) -> str:
    return f"{url} (retries={retries})"


def is_str_list(values: list[object]) -> TypeGuard[list[str]]:
    # 3.10: TypeGuard - user-defined type narrowing function.
    return all(isinstance(v, str) for v in values)


@dataclasses.dataclass(slots=True, kw_only=True, frozen=True)
class Event:
    # 3.10: dataclass(slots=True, kw_only=True) - memory-efficient,
    # keyword-only construction; match_args enables positional class patterns.
    kind: str
    payload: dict[str, Any] = dataclasses.field(default_factory=dict)


class Shape:
    pass


@dataclasses.dataclass
class Circle(Shape):
    radius: float


@dataclasses.dataclass
class Rect(Shape):
    w: float
    h: float


def area(shape: Shape) -> float:
    # 3.10: structural pattern matching - class patterns with positional
    # sub-patterns (via __match_args__), guards, captures and wildcards.
    match shape:
        case Circle(radius=r):
            return math.pi * r * r
        case Rect(w, h) if w == h:
            return w * w
        case Rect(w, h):
            return w * h
        case _:
            raise TypeError(shape)


def handle_command(cmd: object) -> str:
    match cmd:
        case {"action": "move", "to": [x, y]}:          # mapping + sequence patterns
            return f"move to {x},{y}"
        case {"action": "say", "text": str(text)}:      # class pattern on builtins
            return f"say {text!r}"
        case [first, *rest]:                             # sequence with star capture
            return f"list starting with {first}, {len(rest)} more"
        case int() | float() as number if number < 0:   # or-pattern, as-binding, guard
            return "negative number"
        case None:
            return "nothing"
        case _:
            return "unknown"


class Color(enum.StrEnum):
    # 3.11: StrEnum - members are real strings.
    RED = enum.auto()
    GREEN = enum.auto()


class Builder:
    def __init__(self) -> None:
        self.parts: list[str] = []

    # 3.11: typing.Self - "returns an instance of the current class".
    def add(self, part: str) -> Self:
        self.parts.append(part)
        return self


def run_query(sql: LiteralString) -> str:
    # 3.11: LiteralString - type checkers reject strings built from user input
    # (SQL-injection protection).
    return f"executing: {sql}"


def demo_3_10_to_3_11() -> None:
    section("Python 3.10 - 3.11")

    print(f"pattern matching: circle={area(Circle(1)):.3f} square={area(Rect(2, 2))} rect={area(Rect(2, 5))}")
    for cmd in [{"action": "move", "to": [3, 4]}, {"action": "say", "text": "hi"}, [1, 2, 3], -7.5, None, "?"]:
        print(f"  match {cmd!r:40} -> {handle_command(cmd)}")

    # 3.10: X | Y union types, usable at runtime too.
    def parse(value: int | str | None) -> int:
        return int(value) if value is not None else 0
    print(f"union types: {parse('5') + parse(None)}, isinstance(3, int | str) = {isinstance(3, int | str)}")

    # 3.10: parenthesized context managers spanning several lines.
    with (
        contextlib.nullcontext("first") as one,
        contextlib.nullcontext("second") as two,
    ):
        print(f"parenthesized context managers: {one}, {two}")

    # 3.10: zip(strict=True), itertools.pairwise, int.bit_count, aiter/anext.
    try:
        list(zip([1, 2], [1], strict=True))
    except ValueError as e:
        print(f"zip(strict=True): {e}")
    print(f"pairwise: {list(itertools.pairwise('abcd'))}, (255).bit_count() = {(255).bit_count()}")

    add(2, 3)
    print(f"Concatenate: {download('https://example.org')}")
    print(f"TypeGuard: {is_str_list(['a', 'b'])}, {is_str_list(['a', 1])}")
    print(f"dataclass slots/kw_only: {Event(kind='click')}, has __slots__: {hasattr(Event, '__slots__')}")
    print(f"statistics.correlation: {statistics.correlation([1, 2, 3], [2, 4, 7]):.3f}, "
          f"linear_regression slope: {statistics.linear_regression([1, 2, 3], [2, 4, 6]).slope}")

    # 3.11: exception groups and except* - handle several concurrent errors.
    try:
        raise ExceptionGroup("batch failed", [ValueError("bad value"), TypeError("bad type"), ValueError("again")])
    except* ValueError as group:
        print(f"except* ValueError: {len(group.exceptions)} error(s)")
    except* TypeError as group:
        print(f"except* TypeError: {[str(e) for e in group.exceptions]}")

    # 3.11: add_note() attaches extra context to an exception.
    try:
        try:
            1 / 0
        except ZeroDivisionError as e:
            e.add_note("while computing the average of an empty list")
            raise
    except ZeroDivisionError as e:
        print(f"add_note: {e.__notes__}")

    # 3.11: asyncio.TaskGroup (structured concurrency) and asyncio.timeout.
    async def job(n: int) -> int:
        await asyncio.sleep(0.001 * n)
        return n * n
    async def run_group() -> list[int]:
        async with asyncio.timeout(2):
            async with asyncio.TaskGroup() as tg:
                tasks = [tg.create_task(job(i)) for i in range(4)]
        return [t.result() for t in tasks]
    print(f"TaskGroup + timeout: {asyncio.run(run_group())}")

    # 3.11: tomllib - TOML parsing in the standard library.
    config = tomllib.loads('[server]\nhost = "localhost"\nports = [8000, 8001]\n')
    print(f"tomllib: {config['server']}")

    print(f"StrEnum: {Color.RED} == 'red': {Color.RED == 'red'}")
    print(f"Self: {Builder().add('a').add('b').parts}, LiteralString: {run_query('SELECT 1')}")

    # 3.11: Never / assert_never for exhaustiveness checking.
    def check(value: Literal["a", "b"]) -> str:
        if value == "a":
            return "A"
        elif value == "b":
            return "B"
        else:
            assert_never(value)
    print(f"assert_never exhaustiveness: {check('b')}")

    # 3.11: Required/NotRequired in TypedDict, operator.call, datetime.UTC,
    # contextlib.chdir, math.cbrt / exp2.
    class Options(TypedDict, total=False):
        name: Required[str]
        verbose: NotRequired[bool]
    opts: Options = {"name": "demo"}
    with tempfile.TemporaryDirectory() as tmp, contextlib.chdir(tmp):
        inside = pathlib.Path.cwd().name == pathlib.Path(tmp).name
    print(f"Required/NotRequired: {opts}, operator.call: {operator.call(len, 'abc')}, "
          f"UTC: {dt.datetime(2026, 1, 1, tzinfo=dt.UTC).tzname()}, contextlib.chdir worked: {inside}, "
          f"cbrt(27)={math.cbrt(27)} exp2(10)={math.exp2(10)}")


# #############################################################################
#  Python 3.12 - 3.13 (2023 - 2024)
# #############################################################################

# 3.12: PEP 695 - type parameter syntax for generic functions and classes,
# and the "type" statement for (lazily evaluated) type aliases.
type Number = int | float
type Pair[T] = tuple[T, T]
type JsonValue = dict[str, JsonValue] | list[JsonValue] | str | int | float | bool | None


def first[T](items: list[T]) -> T:
    return items[0]


class Stack[T]:
    def __init__(self) -> None:
        self._items: list[T] = []

    def push(self, item: T) -> None:
        self._items.append(item)

    def pop(self) -> T:
        return self._items.pop()


# 3.13: type parameter defaults (PEP 696).
class Box[T = int]:
    def __init__(self, value: T) -> None:
        self.value = value


class BaseRepo:
    def save(self) -> str:
        return "base save"


class SqlRepo(BaseRepo):
    # 3.12: @override - type checkers verify that the method really overrides.
    @override
    def save(self) -> str:
        return "sql save"


# 3.13: warnings.deprecated - runtime DeprecationWarning + type checker support.
@warnings.deprecated("use new_api() instead")
def old_api() -> str:
    return "old result"


def is_int(value: object) -> TypeIs[int]:
    # 3.13: TypeIs - like TypeGuard but also narrows the negative branch.
    return isinstance(value, int)


class Config(TypedDict):
    # 3.13: ReadOnly TypedDict items.
    name: ReadOnly[str]
    level: int


def demo_3_12_to_3_13() -> None:
    section("Python 3.12 - 3.13")

    number: Number = 3.5
    pair: Pair[str] = ("a", "b")
    doc: JsonValue = {"items": [1, "two", None]}
    print(f"type statement: Number={Number.__value__}, Pair[str]={pair}, recursive alias ok: {bool(doc)}")
    s = Stack[int]()
    s.push(1)
    s.push(2)
    print(f"PEP 695 generics: first={first(['x', 'y'])}, Stack.pop={s.pop()}, type params={Stack.__type_params__}, number={number}")
    print(f"type param defaults: Box() default={Box.__type_params__[0].__default__}, Box('s').value={Box('s').value}")

    # 3.12: PEP 701 f-strings - reuse the same quotes, nest arbitrarily,
    # backslashes and comments inside replacement fields.
    words = ["alpha", "beta"]
    print(f"PEP 701 f-strings: {", ".join(words)} | {f"{f"{len(words)}"}"} | {"\n".join(words).count("\n")} newline(s) | {
        sum(len(w) for w in words)  # a comment inside an f-string
    } letters")

    print(f"@override: {SqlRepo().save()}")

    # 3.12: itertools.batched (strict=True added in 3.13), math.sumprod,
    # Path.walk, int.is_integer.
    print(f"batched: {list(itertools.batched(range(7), 3))}, sumprod: {math.sumprod([1, 2, 3], [4, 5, 6])}")
    with tempfile.TemporaryDirectory() as tmp:
        root = pathlib.Path(tmp)
        (root / "sub").mkdir()
        (root / "sub" / "file.txt").write_text("hello")
        walked = [(d.name, sorted(f)) for d, _, f in root.walk()]
        print(f"Path.walk: {walked[-1]}")

    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        result = old_api()
    print(f"@deprecated: {result}, warning: {caught[0].category.__name__}: {caught[0].message}")

    value: int | str = 5
    if is_int(value):
        print(f"TypeIs narrowing: {value + 1}")
    cfg: Config = {"name": "app", "level": 2}
    print(f"ReadOnly TypedDict: {cfg}")

    # 3.13: copy.replace - generic "replace fields" for dataclasses,
    # namedtuples, datetime objects...
    v = Version(2, 1)
    print(f"copy.replace: {copy.replace(v, minor=5)}, date: {copy.replace(dt.date(2026, 1, 31), month=3)}")

    # 3.13: math.fma, statistics.kde, os.process_cpu_count, base64.z85,
    # queue.Queue.shutdown().
    import queue
    q: queue.Queue[int] = queue.Queue()
    q.put(1)
    q.shutdown()
    try:
        q.put(2)
    except queue.ShutDown:
        print(f"Queue.shutdown: put after shutdown rejected (qsize={q.qsize()})")
    density = statistics.kde([1.0, 2.0, 2.5], h=0.5)
    print(f"math.fma(2,3,4)={math.fma(2, 3, 4)}, kde(2.0)={density(2.0):.3f}, "
          f"process_cpu_count>=1: {(os.process_cpu_count() or 1) >= 1}, z85: {base64.z85encode(b'1234')}")

    # 3.13: free-threaded build (no GIL, experimental in 3.13, supported in 3.14).
    gil = getattr(sys, "_is_gil_enabled", lambda: True)()
    print(f"GIL enabled in this interpreter: {gil}")

    # Also 3.13: new interactive REPL with colors and multi-line editing,
    # experimental JIT compiler (PEP 744), well-defined locals() (PEP 667).


# #############################################################################
#  Python 3.14 (October 2025)
# #############################################################################
def make_user(name: str, profile: UserProfile) -> UserProfile:
    # 3.14: deferred evaluation of annotations (PEP 649/749). "UserProfile"
    # is defined BELOW this function and no quotes are needed: annotations
    # are evaluated lazily, only when someone asks for them.
    return profile


@dataclasses.dataclass
class UserProfile:
    name: str


def html_render(template: string.templatelib.Template) -> str:
    # 3.14: template strings (PEP 750). t"..." produces a Template object with
    # the static strings and the interpolations kept separate, so the
    # consumer decides how to combine them - here: HTML-escaping all values.
    import html
    out = []
    for part in template:
        if isinstance(part, str):
            out.append(part)
        else:
            value = format(part.value, part.format_spec)
            out.append(html.escape(value))
    return "".join(out)


def interpreter_task(n: int) -> int:
    return sum(i * i for i in range(n))


def demo_3_14() -> None:
    section("Python 3.14")

    # Template strings
    user_input = "<script>alert('x')</script>"
    template = t"<p>Hello {user_input}!</p>"
    print(f"t-string type: {type(template).__name__}, strings={template.strings}")
    print(f"t-string rendered safely: {html_render(template)}")

    # Deferred annotations + annotationlib
    print(f"deferred annotations: {annotationlib.get_annotations(make_user)}")
    print(f"annotationlib STRING format: {annotationlib.get_annotations(make_user, format=annotationlib.Format.STRING)}")

    # PEP 758: except / except* without parentheses (when no "as" is used).
    try:
        int("not a number")
    except ValueError, TypeError:
        print("except without parentheses: caught ValueError")

    # PEP 765: return/break/continue that exit a "finally" block now emit a
    # SyntaxWarning (they silently swallowed exceptions) - not shown here.

    # concurrent.interpreters (PEP 734): multiple isolated interpreters in one
    # process, each with its own GIL - true parallelism for CPU-bound Python.
    interp = interpreters.create()
    interp.exec("import math; secret = math.factorial(10)")
    print(f"subinterpreter call: {interp.call(interpreter_task, 1000)}")
    interp.close()
    with InterpreterPoolExecutor(max_workers=2) as pool:
        print(f"InterpreterPoolExecutor: {list(pool.map(interpreter_task, [10, 100]))}")

    # compression.zstd (PEP 784): Zstandard compression in the stdlib.
    raw = b"modern python " * 100
    packed = zstd.compress(raw)
    print(f"zstd: {len(raw)} -> {len(packed)} bytes, round-trip ok: {zstd.decompress(packed) == raw}")

    # uuid.uuid7 (time-ordered UUIDs), heapq max-heap functions,
    # functools.Placeholder, map(strict=True), date.strptime,
    # float.from_number, operator.is_none.
    u = uuid.uuid7()
    heap = [3, 9, 1, 7]
    heapq.heapify_max(heap)
    square_of = functools.partial(pow, functools.Placeholder, 2)
    print(f"uuid7 version: {u.version}, heapify_max top: {heap[0]}, Placeholder: {square_of(12)}")
    print(f"map(strict=True): {list(map(operator.add, [1, 2], [10, 20], strict=True))}, "
          f"date.strptime: {dt.date.strptime('2025-10-07', '%Y-%m-%d')}, "
          f"float.from_number(3): {float.from_number(3)}, operator.is_none(None): {operator.is_none(None)}")

    # pathlib: Path.copy / Path.move / Path.info (3.14)
    with tempfile.TemporaryDirectory() as tmp:
        src = pathlib.Path(tmp) / "a.txt"
        src.write_text("data")
        dst = src.copy(pathlib.Path(tmp) / "b.txt")
        moved = dst.move(pathlib.Path(tmp) / "c.txt")
        print(f"Path.copy/move: {moved.name} exists={moved.info.exists()} is_file={moved.info.is_file()}")

    # asyncio introspection: capture and print the async call graph
    # (also available from the command line: python -m asyncio ps PID).
    async def leaf() -> int:
        graph = asyncio.capture_call_graph()
        return len(graph.call_stack) if graph else 0
    async def root() -> int:
        return await asyncio.create_task(leaf())
    print(f"asyncio.capture_call_graph: {asyncio.run(root())} frame(s) on the call stack")

    # contextvars.Token as a context manager: automatic reset.
    with request_id.set("temporary"):
        inside = request_id.get()
    print(f"ContextVar token as context manager: inside={inside}, after={request_id.get()}")

    # Also in 3.14: free-threaded Python officially supported (PEP 779),
    # syntax highlighting in the REPL, remote debugging (pdb -p PID),
    # colored argparse/unittest/json output, Sigstore release signing.


# #############################################################################
#  Python 3.15 (October 2026)
# #############################################################################
class Movie(TypedDict, closed=True):
    # PEP 728: "closed" TypedDicts allow no keys besides the declared ones.
    title: str
    year: int


class Labels(TypedDict, extra_items=str):
    # PEP 728: extra keys are allowed, but their values must be str.
    name: str


@disjoint_base
class Node:
    # PEP 800: @disjoint_base tells type checkers that no class can inherit
    # from both this class and another disjoint base (lets them prove that
    # certain isinstance() checks or intersections are impossible).
    __slots__ = ("value",)

    def __init__(self, value: int) -> None:
        self.value = value


def parse_as[T](form: TypeForm[T], raw: str) -> T:
    # PEP 747: TypeForm[T] annotates a parameter that receives a *type
    # expression* (like int, list[str], int | None) - useful for
    # validators, ORMs and serializers.
    return typing.cast(T, json.loads(raw))


def demo_3_15() -> None:
    section("Python 3.15")

    # Lazy imports (top of file)
    print(f"lazy import: difflib loaded at start = {DIFFLIB_LOADED_AT_START}")
    ratio = difflib.SequenceMatcher(None, "python", "pythons").ratio()   # first use -> import
    print(f"lazy import: now loaded = {'difflib' in sys.modules}, similarity = {ratio:.3f}")
    print(f"lazy from-import: {Fraction(3, 4) + Fraction(1, 4)}, sys.get_lazy_imports() = {sys.get_lazy_imports()!r}")
    # Also: a module-level __lazy_modules__ = ["json", ...] list makes ordinary
    # "import json" statements lazy, staying compatible with older Pythons.

    # frozendict (PEP 814): immutable, hashable dictionary built-in.
    settings = frozendict(host="localhost", port=8080)
    cache = {settings: "connection #1"}            # usable as a dict key / set member
    updated = settings | {"port": 9090}            # operators return new frozendicts
    try:
        settings["port"] = 1                       # type: ignore[index]
    except TypeError as e:
        print(f"frozendict is immutable: {type(e).__name__}")
    print(f"frozendict: {settings}, as key: {cache[settings]}, merged: {updated}, hash ok: {hash(settings) == hash(frozendict(settings))}")

    # sentinel (PEP 661): unique named marker objects with a clean repr,
    # replacing ad-hoc "_MISSING = object()" idioms.
    MISSING = sentinel("MISSING")

    def get_option(name: str, default: object = MISSING) -> object:
        if default is MISSING:
            return f"{name}: no default given ({MISSING!r})"
        return f"{name}: default={default}"
    print(f"sentinel: {get_option('timeout')} | {get_option('retries', 3)}")

    # Unpacking in comprehensions (PEP 798): [*x for x in ...] flattens one level.
    groups = [[1, 2], [3], [4, 5, 6]]
    dicts = [{"a": 1}, {"b": 2}, {"a": 3}]
    print(f"[*g for g in groups] = {[*g for g in groups]}, {{**d for d in dicts}} = { {**d for d in dicts} }")
    print(f"set/generator unpacking: {sorted({*g for g in groups})}, sum = {sum((*g for g in groups))}")

    # UTF-8 mode by default (PEP 686): open(), Path.read_text() etc. use UTF-8
    # unless an encoding is given - no more locale-dependent surprises on Windows.
    with tempfile.TemporaryDirectory() as tmp:
        f = pathlib.Path(tmp) / "unicode.txt"
        f.write_text("Gr\u00fc\u00dfe \u2713")
        print(f"UTF-8 default: utf8_mode={sys.flags.utf8_mode}, read back {len(f.read_text())} chars, {len(f.read_bytes())} bytes")

    # TypedDict closed / extra_items (PEP 728)
    movie: Movie = {"title": "Solaris", "year": 1972}
    labels: Labels = {"name": "svc", "team": "core", "tier": "gold"}
    print(f"closed TypedDict: {Movie.__closed__}, extra_items type: {Labels.__extra_items__.__name__}, values: {movie} {labels}")

    print(f"TypeForm: {parse_as(list[int], '[1, 2, 3]')}, disjoint_base Node: {Node(1).value}")

    # math.integer (PEP 791): integer-specific functions in their own module.
    print(f"math.integer: isqrt(10**20+1)={math.integer.isqrt(10**20 + 1)}, comb(52, 5)={math.integer.comb(52, 5)}")

    # slice is now subscriptable for typing: slice[int]
    window: slice[int] = slice(1, 4)
    print(f"slice[int] annotation: {[10, 20, 30, 40, 50][window]}")

    # Also in 3.15: the Tachyon sampling profiler (python -m profiling.sampling,
    # PEP 799), a much faster JIT, and a stable ABI for free-threaded builds.


# #############################################################################
def main() -> None:
    print(f"Modern Python showcase (3.2 .. 3.15) running on {sys.version.split()[0]}")
    demo_3_2_to_3_5()
    demo_3_6_to_3_7()
    demo_3_8_to_3_9()
    demo_3_10_to_3_11()
    demo_3_12_to_3_13()
    demo_3_14()
    demo_3_15()
    print("\nDone.")


if __name__ == "__main__":
    main()
