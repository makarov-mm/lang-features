#!/usr/bin/env dotnet
#:property LangVersion=14.0
#:property Nullable=enable
#:property AllowUnsafeBlocks=true
#:property PublishAot=false
// ============================================================================
//  ModernCSharpShowcase.cs
//
//  A single-file tour of C# from C# 5 (2012) to C# 14 (.NET 10, Nov 2025),
//  plus the most important .NET library additions of the same period.
//  Every feature is commented: which version introduced it, how it works,
//  and what it is used for.
//
//  How to run - no .csproj, no .sln needed:
//      dotnet run ModernCSharpShowcase.cs          (.NET 10 SDK or newer)
//  Visual Studio 2026 opens and runs this file directly as well.
//
//  The lines starting with "#:" at the top are .NET 10 "file-based app"
//  directives. They replace what used to live in a .csproj:
//    #:property LangVersion=14.0      -> C# language version
//    #:property Nullable=enable       -> nullable reference types (C# 8)
//    #:property AllowUnsafeBlocks=true-> needed for pointers/function pointers
//    #:property PublishAot=false      -> file-based apps default to Native AOT;
//                                        we turn it off so reflection-based
//                                        demos produce no AOT-analysis warnings
//  The "#!" shebang line lets you run the file directly on Unix:
//      chmod +x ModernCSharpShowcase.cs && ./ModernCSharpShowcase.cs
// ============================================================================

// global using (C# 10): applies to every file of the compilation. File-based
// apps also get "implicit usings" (System, System.Linq, System.IO,
// System.Collections.Generic, System.Threading.Tasks, ...) automatically.
global using System.Text;

using System.Buffers;
using System.Buffers.Text;
using System.Collections.Frozen;
using System.Collections.ObjectModel;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text.RegularExpressions;
using System.Threading.Channels;
// using static (C# 6): import static members of a type.
using static System.Math;
// Alias any type (C# 12): aliases for tuples, arrays, pointers, etc.
using Point3 = (double X, double Y, double Z);

// ----------------------------------------------------------------------------
// Top-level statements (C# 9): the program's entry point without a class or
// a Main method. "await" here makes the implicit Main async (async Main was
// added in C# 7.1).
// ----------------------------------------------------------------------------
Console.WriteLine($"Modern C# showcase - C# 5 .. C# 14 on .NET {Environment.Version}");
Console.WriteLine($"Module initializer ran before Main: {Startup.InitializedAt is not null}");

await CSharp5And6.RunAsync();
CSharp7.Run();
await CSharp8.RunAsync();
CSharp9.Run();
CSharp10.Run();
CSharp11.Run();
CSharp12.Run();
CSharp13.Run();
CSharp14.Run();
await ModernLibrary.RunAsync();

Console.WriteLine();
Console.WriteLine("Done.");

// Helper used by all sections. (Types and functions declared after top-level
// statements are allowed; the statements must come first.)
static class Out
{
    public static void Section(string title) => Console.WriteLine($"\n=== {title} ===");
}

// Module initializers (C# 9): run once when the assembly is loaded, before
// any other code - useful for registering serializers, plugins, etc.
static class Startup
{
    public static DateTime? InitializedAt;

    [ModuleInitializer]
    internal static void Init() => InitializedAt = DateTime.UtcNow;
}

// ############################################################################
//  C# 5 (2012) and C# 6 (2015)
// ############################################################################
static class CSharp5And6
{
    // C# 5: async/await. An async method returns Task/Task<T>; "await"
    // suspends the method without blocking the thread and resumes it when the
    // awaited operation completes.
    static async Task<int> ComputeAsync(int x)
    {
        await Task.Delay(10);
        return x * 2;
    }

    // C# 5: caller info attributes - the compiler fills in the call site.
    static string Where([CallerMemberName] string member = "",
                        [CallerLineNumber] int line = 0) => $"{member}:{line}";

    // C# 6: auto-property initializers and getter-only auto-properties.
    class Order
    {
        public int Id { get; } = 1;                       // getter-only (immutable after ctor)
        public List<string> Items { get; set; } = new();  // initializer

        // C# 6: expression-bodied members (methods and read-only properties).
        public int Count => Items.Count;
        public override string ToString() => $"Order {Id} with {Count} item(s)";
    }

    class Customer
    {
        public string? Name { get; set; }
        public Customer? Referrer { get; set; }
    }

    public static async Task RunAsync()
    {
        Out.Section("C# 5 / C# 6");

        Console.WriteLine($"async/await: {await ComputeAsync(21)}");
        var parallelResults = await Task.WhenAll(ComputeAsync(1), ComputeAsync(2), ComputeAsync(3));
        Console.WriteLine($"Task.WhenAll: {string.Join(", ", parallelResults)}");
        Console.WriteLine($"caller info: {Where()}");

        // C# 6: string interpolation $"..." with format specifiers and alignment.
        double price = 1234.5;
        Console.WriteLine($"interpolation: [{price,12:N2}] [{DateTime.MinValue:yyyy}]");

        // C# 6: nameof - the name of a symbol as a string, refactoring-safe.
        Console.WriteLine($"nameof: {nameof(Order)}.{nameof(Order.Items)}");

        // C# 6: null-conditional operators ?. and ?[] (+ the null-coalescing ??).
        Customer? c = new() { Name = "Ann" };
        Customer? nobody = null;
        Console.WriteLine($"null-conditional: {c?.Referrer?.Name ?? "(no referrer)"}, {nobody?.Name?.Length ?? -1}");
        string[]? arr = null;
        Console.WriteLine($"?[] on null array: {arr?[0] ?? "null"}");

        // C# 6: index initializers.
        var lookup = new Dictionary<string, int> { ["one"] = 1, ["two"] = 2 };
        Console.WriteLine($"index initializer: {lookup["two"]}");

        var order = new Order { Items = { "pen", "ink" } };
        Console.WriteLine($"auto-property init / expression-bodied: {order}");

        // C# 6: using static (Math imported at the top: Sqrt instead of Math.Sqrt).
        Console.WriteLine($"using static: Sqrt(2) = {Sqrt(2):F4}, PI = {PI:F3}");

        // C# 6: exception filters with "when", and await inside catch/finally.
        try
        {
            throw new InvalidOperationException("code 42");
        }
        catch (InvalidOperationException ex) when (ex.Message.Contains("42"))
        {
            await Task.Yield();
            Console.WriteLine($"exception filter caught: {ex.Message}");
        }
        finally
        {
            await Task.CompletedTask;
        }
    }
}

// ############################################################################
//  C# 7.0 - 7.3 (2017 - 2018)
// ############################################################################
static class CSharp7
{
    // Tuples with named elements (value types: System.ValueTuple).
    static (int Min, int Max, double Average) Stats(IEnumerable<int> values)
    {
        var list = values.ToList();
        return (list.Min(), list.Max(), list.Average());
    }

    // ref returns / ref locals: return a reference to storage, not a copy.
    static ref int FindFirstNegative(int[] numbers)
    {
        for (int i = 0; i < numbers.Length; i++)
            if (numbers[i] < 0) return ref numbers[i];
        throw new InvalidOperationException("none");   // C# 7: throw expressions/statements anywhere
    }

    // ValueTask (generalized async return types): no allocation when the
    // result is already available synchronously.
    static async ValueTask<int> CachedAsync(bool cached)
    {
        if (cached) return 7;
        await Task.Delay(1);
        return 8;
    }

    // C# 7.2: "in" parameters (read-only by-reference, avoids copying big structs)
    // and readonly struct (all fields immutable - the compiler avoids defensive copies).
    readonly struct BigVector(double x, double y, double z)
    {
        public double X { get; } = x;
        public double Y { get; } = y;
        public double Z { get; } = z;
    }
    static double Length(in BigVector v) => Sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);

    // C# 7.2: ref struct - must live on the stack (like Span<T>).
    ref struct StackOnlyParser
    {
        private ReadOnlySpan<char> _rest;
        public StackOnlyParser(ReadOnlySpan<char> text) => _rest = text;
        public bool TryNext(out ReadOnlySpan<char> token)
        {
            _rest = _rest.TrimStart(' ');
            if (_rest.IsEmpty) { token = default; return false; }
            int idx = _rest.IndexOf(' ');
            token = idx < 0 ? _rest : _rest[..idx];
            _rest = idx < 0 ? default : _rest[idx..];
            return true;
        }
    }

    // C# 7.2: private protected - accessible from derived classes in the same assembly.
    class BaseWidget { private protected int Secret = 3; }
    class DerivedWidget : BaseWidget { public int Reveal() => Secret * 2; }

    // C# 7.3: generic constraints unmanaged, Enum, Delegate.
    static unsafe int SizeOf<T>() where T : unmanaged => sizeof(T);
    static string[] EnumNames<T>() where T : struct, Enum => Enum.GetNames<T>();

    enum Weekday { Mon, Tue, Wed }

    // Deconstruct method: makes any type usable in deconstruction.
    class Person(string first, string last)
    {
        public void Deconstruct(out string f, out string l) => (f, l) = (first, last);
    }

    public static void Run()
    {
        Out.Section("C# 7.x");

        // Tuples and deconstruction.
        var (min, max, avg) = Stats([4, 8, 15, 16, 23, 42]);
        Console.WriteLine($"tuples + deconstruction: min={min} max={max} avg={avg:F2}");
        var (first, _) = new Person("Ada", "Lovelace");   // "_" is a discard
        Console.WriteLine($"custom Deconstruct + discard: {first}");

        // C# 7.1: inferred tuple element names; C# 7.3: tuple equality.
        int count = 3; string label = "x";
        var inferred = (count, label);
        Console.WriteLine($"inferred names: {inferred.count}/{inferred.label}, tuple ==: {(1, "a") == (1, "a")}");

        // Pattern matching: type patterns in "is" and in switch with "when".
        object[] things = [42, "text", 3.5, null!];
        foreach (var o in things)
        {
            string description = "";
            switch (o)
            {
                case int i when i > 40: description = $"big int {i}"; break;
                case string s: description = $"string of {s.Length}"; break;
                case null: description = "null"; break;
                default: description = o.GetType().Name; break;
            }
            Console.Write($"[{description}] ");
        }
        Console.WriteLine("<- switch patterns");
        if (things[0] is int n && n % 2 == 0) Console.WriteLine($"'is' pattern: {n} is even");

        // out variables declared inline.
        if (int.TryParse("123", out var parsed)) Console.WriteLine($"out var: {parsed}");

        // Local functions (can be recursive, capture locals, be iterators).
        static long Fact(int k) => k <= 1 ? 1 : k * Fact(k - 1);
        Console.WriteLine($"local function: 20! = {Fact(20)}");

        // ref locals & returns
        int[] data = [5, -3, 7];
        ref int slot = ref FindFirstNegative(data);
        slot = 0;                                           // writes into the array
        Console.WriteLine($"ref return: [{string.Join(", ", data)}]");

        // Binary literals and digit separators (C# 7.0), leading separator (7.2).
        int flags = 0b_0101_0101;
        long big = 1_000_000_000_000;
        Console.WriteLine($"binary literal: {flags}, digit separators: {big:N0}");

        // Throw expressions.
        string? maybe = "value";
        string sure = maybe ?? throw new ArgumentNullException(nameof(maybe));
        Console.WriteLine($"throw expression passed: {sure}");

        Console.WriteLine($"ValueTask: {CachedAsync(true).Result}");

        // C# 7.1: default literal.
        int zero = default;
        string? none = default;
        Console.WriteLine($"default literal: {zero} {none ?? "null"}");

        var v = new BigVector(1, 2, 2);
        Console.WriteLine($"in parameter + readonly struct: |v| = {Length(in v)}");

        // C# 7.2: non-trailing named arguments.
        Console.WriteLine($"non-trailing named args: {string.Join(separator: "-", "a", "b")}");

        var parser = new StackOnlyParser("ref struct tokenizer demo");
        int tokens = 0;
        while (parser.TryNext(out _)) tokens++;
        Console.WriteLine($"ref struct parser: {tokens} tokens");

        Console.WriteLine($"private protected: {new DerivedWidget().Reveal()}");
        Console.WriteLine($"unmanaged constraint: sizeof(decimal)={SizeOf<decimal>()}, Enum constraint: {string.Join("/", EnumNames<Weekday>())}");

        // C# 7.3: stackalloc with initializer (assigned to Span - no unsafe needed).
        Span<int> primes = stackalloc int[] { 2, 3, 5, 7 };
        Console.WriteLine($"stackalloc initializer: sum = {primes[0] + primes[1] + primes[2] + primes[3]}");

        // C# 7.2: conditional ref expression.
        int a = 1, b = 2;
        ref int bigger = ref (a > b ? ref a : ref b);
        bigger = 100;
        Console.WriteLine($"conditional ref: a={a} b={b}");
    }
}

// ############################################################################
//  C# 8 (2019, .NET Core 3)
// ############################################################################
static class CSharp8
{
    // Default interface methods: interfaces may contain implementations,
    // so new members can be added without breaking implementers.
    interface ILogger
    {
        void Write(string message);
        void Info(string message) => Write($"[info] {message}");
    }
    class ConsoleLogger : ILogger
    {
        public void Write(string message) => Console.WriteLine(message);
    }

    // Async streams: IAsyncEnumerable<T> produced with "yield return" inside
    // an async method, consumed with "await foreach".
    static async IAsyncEnumerable<int> TicksAsync(int count)
    {
        for (int i = 1; i <= count; i++)
        {
            await Task.Delay(1);
            yield return i;
        }
    }

    // Readonly members on mutable structs.
    struct Counter
    {
        public int Value;
        public readonly bool IsZero => Value == 0;   // cannot modify state
    }

    record Address(string City, string Country);
    record Shopper(string Name, int Age, Address Home);

    // Switch expressions with property, tuple and positional patterns.
    static string Classify(Shopper s) => s switch
    {
        { Age: < 18 } => "minor",
        { Home: { Country: "DE" } } => "German adult",
        { Name: var name } when name.StartsWith('Z') => "Z-person",
        _ => "other adult",
    };

    static string RockPaperScissors(string a, string b) => (a, b) switch
    {
        ("rock", "scissors") or ("scissors", "paper") or ("paper", "rock") => "first wins",
        var (x, y) when x == y => "tie",
        _ => "second wins",
    };

    public static async Task RunAsync()
    {
        Out.Section("C# 8");

        // Nullable reference types: string is non-nullable, string? may be null;
        // the compiler warns on possible null dereferences.
        string? maybeNull = Environment.GetEnvironmentVariable("SURELY_NOT_DEFINED_42");
        Console.WriteLine($"nullable reference types: length = {maybeNull?.Length ?? 0}");

        // Indices and ranges: ^1 = last element, a..b = slice.
        int[] nums = [0, 1, 2, 3, 4, 5, 6, 7];
        Console.WriteLine($"indices/ranges: last={nums[^1]}, [2..5]=[{string.Join(",", nums[2..5])}], [^3..]=[{string.Join(",", nums[^3..])}]");
        Console.WriteLine($"string range: {"Hello, world"[7..^0]}");

        // Switch expressions and recursive patterns.
        var shoppers = new[]
        {
            new Shopper("Tom", 15, new("Paris", "FR")),
            new Shopper("Uwe", 40, new("Berlin", "DE")),
            new Shopper("Zed", 33, new("Oslo", "NO")),
        };
        foreach (var s in shoppers) Console.WriteLine($"switch expression: {s.Name} -> {Classify(s)}");
        Console.WriteLine($"tuple patterns: {RockPaperScissors("rock", "scissors")}, {RockPaperScissors("paper", "paper")}");

        // using declarations: disposed at the end of the enclosing scope.
        using var ms = new MemoryStream();
        ms.WriteByte(1);
        Console.WriteLine($"using declaration: stream length {ms.Length}");

        // Static local functions cannot capture state (no hidden allocations).
        static int Twice(int x) => x * 2;
        Console.WriteLine($"static local function: {Twice(21)}");

        // Null-coalescing assignment ??=
        List<string>? cache = null;
        cache ??= new List<string>();
        cache.Add("filled");
        Console.WriteLine($"??= : {cache[0]}");

        ILogger log = new ConsoleLogger();
        log.Info("default interface method");

        await foreach (var t in TicksAsync(3)) Console.Write($"tick {t} ");
        Console.WriteLine("<- await foreach");

        var counter = new Counter { Value = 0 };
        Console.WriteLine($"readonly struct member: IsZero={counter.IsZero}");

        // stackalloc in nested expressions.
        int index = MemoryExtensions.IndexOf(stackalloc int[] { 10, 20, 30 }, 20);
        Console.WriteLine($"nested stackalloc: index of 20 = {index}");

        // Interpolated verbatim strings can be written $@ or @$.
        string path = @$"C:\Users\{Environment.UserName.Length}\docs";
        Console.WriteLine($"@$ verbatim interpolation: {path}");
    }
}

// ############################################################################
//  C# 9 (2020, .NET 5)
// ############################################################################
static class CSharp9
{
    // Records: reference types with value-based equality, ToString,
    // deconstruction, and non-destructive mutation via "with".
    public record Product(string Name, decimal Price)
    {
        // Init-only setter: assignable in object initializers, immutable afterwards.
        public string Category { get; init; } = "general";
    }

    // Covariant return types: an override may return a more derived type.
    class Animal { public virtual Animal Clone() => new Animal(); }
    class Cat : Animal { public override Cat Clone() => new Cat(); }

    static int Add(int a, int b) => a + b;

    // Function pointers (unsafe): call through raw pointers, no delegate allocation.
    static unsafe int CallThroughPointer()
    {
        delegate*<int, int, int> fp = &Add;
        return fp(40, 2);
    }

    // [SkipLocalsInit]: do not zero stack memory (perf for stackalloc buffers).
    [SkipLocalsInit]
    static int SumFirst(int n)
    {
        Span<int> buffer = stackalloc int[n];
        for (int i = 0; i < n; i++) buffer[i] = i;
        int s = 0;
        foreach (var x in buffer) s += x;
        return s;
    }

    public static void Run()
    {
        Out.Section("C# 9");

        var p1 = new Product("Laptop", 999m) { Category = "tech" };
        var p2 = p1 with { Price = 899m };                     // non-destructive mutation
        var p3 = new Product("Laptop", 999m) { Category = "tech" };
        Console.WriteLine($"records: {p2}");
        Console.WriteLine($"value equality: p1 == p3 is {p1 == p3}, p1 == p2 is {p1 == p2}");
        var (name, price) = p2;
        Console.WriteLine($"record deconstruction: {name} {price}");

        // Target-typed new: the type is inferred from the declaration.
        Dictionary<string, List<int>> index = new();
        Product cheap = new("Pen", 1.5m);
        Console.WriteLine($"target-typed new: {index.Count} / {cheap.Name}");

        // Pattern combinators and relational patterns: and, or, not, <, >=, ...
        static string Temperature(double t) => t switch
        {
            < 0 => "freezing",
            >= 0 and < 15 => "cold",
            >= 15 and < 25 => "pleasant",
            _ => "hot",
        };
        static bool IsLetter(char c) => c is (>= 'a' and <= 'z') or (>= 'A' and <= 'Z');
        object? o = "not null";
        Console.WriteLine($"relational patterns: {Temperature(-3)}, {Temperature(20)}; IsLetter('q')={IsLetter('q')}; 'is not null'={o is not null}");

        // Native-sized integers nint / nuint.
        nint pointerSized = 42;
        Console.WriteLine($"nint: {pointerSized}, size {IntPtr.Size} bytes");

        Console.WriteLine($"function pointer: {CallThroughPointer()}");

        // Static anonymous functions: cannot capture locals (prevents accidental closures).
        Func<int, int> square = static x => x * x;
        // Lambda discard parameters.
        Func<int, int, int> constant = (_, _) => 1;
        Console.WriteLine($"static lambda: {square(9)}, discard params: {constant(5, 6)}");

        Animal a = new Cat().Clone();
        Console.WriteLine($"covariant return: {a.GetType().Name}");

        // Target-typed conditional expression.
        int? maybe = DateTime.Now.Year > 2000 ? 1 : null;
        Console.WriteLine($"target-typed ?: : {maybe}");

        Console.WriteLine($"[SkipLocalsInit] stackalloc sum: {SumFirst(10)}");

        // Extension GetEnumerator: foreach over a type via an extension method (see RangeExtensions).
        foreach (var i in 1..4) Console.Write($"{i} ");
        Console.WriteLine("<- foreach over a Range via extension GetEnumerator");
    }
}

// Extension GetEnumerator (C# 9) - lets "foreach (var i in 1..4)" work.
static class RangeExtensions
{
    public static IEnumerator<int> GetEnumerator(this Range r)
    {
        for (int i = r.Start.Value; i < r.End.Value; i++) yield return i;
    }
}

// ############################################################################
//  C# 10 (2021, .NET 6)
// ############################################################################
// Also new in C# 10: file-scoped namespaces ("namespace MyApp;" followed by
// the file's types, saving one indentation level). They cannot be combined
// with top-level statements in the same file, so this demo does not use one.
static class CSharp10
{
    // Record structs: value-type records.
    public readonly record struct Money(decimal Amount, string Currency)
    {
        public override string ToString() => $"{Amount:F2} {Currency}";
    }

    // Structs with parameterless constructors and field initializers.
    struct Settings
    {
        public int Retries = 3;
        public string Mode;
        public Settings() => Mode = "safe";
    }

    // CallerArgumentExpression: receive the source text of an argument
    // (great for guard clauses and assertions).
    static void Ensure(bool condition, [CallerArgumentExpression(nameof(condition))] string? expr = null)
    {
        if (!condition) throw new ArgumentException($"Failed: {expr}");
        Console.WriteLine($"CallerArgumentExpression: checked '{expr}'");
    }

    // Constant interpolated strings (when all holes are constant strings).
    const string Product = "Showcase";
    const string Banner = $"{Product} v10";

    record Employee(string Name, Team Team);
    record Team(string Name, Employee? Lead);

    public static void Run()
    {
        Out.Section("C# 10");

        var price = new Money(19.99m, "EUR");
        var discounted = price with { Amount = 14.99m };
        Console.WriteLine($"record struct: {price} -> {discounted}, equal: {price == discounted}");

        var settings = new Settings();
        Console.WriteLine($"parameterless struct ctor: retries={settings.Retries} mode={settings.Mode}");

        Ensure(price.Amount > 0);
        Console.WriteLine($"const interpolated string: {Banner}");

        // Extended property patterns: { Team.Lead.Name: ... } instead of nesting.
        var boss = new Employee("Grace", new Team("core", null));
        var dev = new Employee("Linus", new Team("core", boss));
        if (dev is { Team.Lead.Name: "Grace" }) Console.WriteLine("extended property pattern: Linus reports to Grace");

        // Lambda improvements: natural type (var), explicit return type, attributes.
        var parse = (string s) => int.Parse(s);                 // inferred Func<string, int>
        var choose = object (bool b) => b ? 1 : "one";          // explicit return type
        var tagged = [Obsolete("demo")] (int x) => x + 1;       // attribute on lambda
        Console.WriteLine($"lambda natural type: {parse("41") + 1}, explicit return: {choose(false)}, attributed: {tagged(1)}");

        // "with" on anonymous types and structs.
        var anon = new { X = 1, Y = 2 };
        var moved = anon with { Y = 5 };
        Console.WriteLine($"with on anonymous type: {moved}");

        // Mixed declaration and assignment in deconstruction.
        int existing;
        (existing, var fresh) = (10, 20);
        Console.WriteLine($"mixed deconstruction: {existing} {fresh}");

        // Interpolated string handlers (C# 10 / .NET 6): $"..." passed to a
        // StringBuilder.Append or to logging APIs is formatted directly into
        // the destination buffer, without allocating a temporary string.
        var sb = new StringBuilder();
        sb.Append($"handler-based append {42:X}");
        Console.WriteLine(sb.ToString());
    }
}

// ############################################################################
//  C# 11 (2022, .NET 7)
// ############################################################################

// Generic math: static abstract members in interfaces. INumber<T> lets one
// generic method work for every numeric type (int, double, decimal, Int128...).
static class GenericMath
{
    public static T Sum<T>(IEnumerable<T> values) where T : INumber<T>
    {
        T total = T.Zero;
        foreach (var v in values) total += v;
        return total;
    }

    // Your own interface with static abstract members.
    public interface IShape<TSelf> where TSelf : IShape<TSelf>
    {
        static abstract string Kind { get; }
        static abstract TSelf Unit();
        double Area { get; }
    }
    public record Square(double Side) : IShape<Square>
    {
        public static string Kind => "square";
        public static Square Unit() => new(1);
        public double Area => Side * Side;
    }
    public static string Describe<T>() where T : IShape<T> => $"{T.Kind} unit area = {T.Unit().Area}";
}

// Generic attributes: attributes can be generic classes.
[AttributeUsage(AttributeTargets.Property)]
sealed class ValidateWithAttribute<TValidator> : Attribute where TValidator : new() { }
sealed class NotEmptyValidator { }

// File-local types: visible only inside this file (no name clashes with
// generated code or other files).
file sealed class FileLocalHelper
{
    public static string Hello() => "hello from a file-local type";
}

static class CSharp11
{
    // Required members: object initializers MUST set them.
    class User
    {
        public required string Login { get; init; }
        [ValidateWith<NotEmptyValidator>]
        public string? Email { get; init; }
    }

    // Ref fields and scoped: a ref struct holding a reference to a variable.
    ref struct RefHolder
    {
        public ref int Target;
        public RefHolder(ref int target) => Target = ref target;
    }
    static int SumScoped(scoped ReadOnlySpan<int> span)   // "scoped": the span cannot escape
    {
        int s = 0;
        foreach (var x in span) s += x;
        return s;
    }

    // Checked user-defined operators: separate implementations for checked/unchecked contexts.
    readonly record struct Tiny(byte Value)
    {
        public static Tiny operator +(Tiny a, Tiny b) => new((byte)(a.Value + b.Value));
        public static Tiny operator checked +(Tiny a, Tiny b) => new(checked((byte)(a.Value + b.Value)));
    }

    // Auto-default structs: fields not assigned in the ctor are zero-initialized.
    struct Measurement
    {
        public double Value;
        public string? Unit { get; set; }
        public Measurement(double v) => Value = v; // Unit is auto-defaulted (null), no compile error anymore
    }

    // Extended nameof scope: method parameters can be referenced with nameof
    // inside attributes applied to that method (here a nullability contract).
    [return: NotNullIfNotNull(nameof(text))]
    static string? Shout(string? text) => text?.ToUpperInvariant();

    public static void Run()
    {
        Out.Section("C# 11");

        // Raw string literals: no escaping; indentation is stripped based on the
        // closing quotes; $$ means interpolation uses {{ }}.
        string json = """
            {
              "name": "raw \"quotes\" stay as-is",
              "path": "C:\temp"
            }
            """;
        int id = 7;
        string templ = $$"""{"id": {{id}}, "braces": "{}"}""";
        Console.WriteLine($"raw string literal:\n{json}\nraw interpolated: {templ}");

        // UTF-8 string literals: "..."u8 is a ReadOnlySpan<byte>, encoded at compile time.
        ReadOnlySpan<byte> utf8 = "Gr\u00FC\u00DFe"u8;
        Console.WriteLine($"u8 literal: {utf8.Length} bytes");

        // Newlines in interpolation holes.
        int[] scores = [3, 9, 4];
        Console.WriteLine($"newlines in holes: {scores
            .Where(s => s > 3)
            .Sum()}");

        // List patterns: match arrays/lists by shape, with slices "..".
        static string Shape(int[] a) => a switch
        {
            [] => "empty",
            [var single] => $"one element {single}",
            [1, .., var last] => $"starts with 1, ends with {last}",
            [_, _, ..] => "two or more",
        };
        Console.WriteLine($"list patterns: {Shape([])}, {Shape([5])}, {Shape([1, 2, 3, 9])}, {Shape([4, 4])}");

        // Pattern matching a Span<char> against a constant string.
        ReadOnlySpan<char> command = "stop".AsSpan();
        Console.WriteLine($"span is \"stop\": {command is "stop"}");

        var u = new User { Login = "neo" };
        Console.WriteLine($"required members: {u.Login}");

        Console.WriteLine($"generic math: {GenericMath.Sum([1, 2, 3])} {GenericMath.Sum([1.5, 2.25])} {GenericMath.Sum<decimal>([0.1m, 0.2m])}");
        Console.WriteLine($"static abstract interface members: {GenericMath.Describe<GenericMath.Square>()}");
        Console.WriteLine($"Int128 (.NET 7): {Int128.MaxValue}");

        Console.WriteLine($"file-local type: {FileLocalHelper.Hello()}");

        int value = 1;
        var holder = new RefHolder(ref value);
        holder.Target = 99;
        Console.WriteLine($"ref field: value = {value}, scoped span sum = {SumScoped([1, 2, 3])}");

        // Unsigned right shift >>> on signed types (logical shift).
        int negative = -16;
        Console.WriteLine($">> vs >>> : {negative >> 2} vs {negative >>> 28}");

        try
        {
            var overflow = checked(new Tiny(200) + new Tiny(100));
            Console.WriteLine(overflow);
        }
        catch (OverflowException)
        {
            Console.WriteLine($"checked operator threw; unchecked gives {(new Tiny(200) + new Tiny(100)).Value}");
        }

        var m = new Measurement(3.5);
        Console.WriteLine($"auto-default struct: {m.Value} unit={(m.Unit ?? "null")}");

        Console.WriteLine($"extended nameof scope: {Shout("hi")} / {Shout(null) ?? "null"}");
        var emailProp = typeof(User).GetProperty(nameof(User.Email))!;
        Console.WriteLine($"generic attribute present: {Attribute.IsDefined(emailProp, typeof(ValidateWithAttribute<NotEmptyValidator>))}");
    }
}

// ############################################################################
//  C# 12 (2023, .NET 8)
// ############################################################################

// Primary constructors for classes and structs: parameters are in scope for
// the whole type body (captured as hidden fields when needed).
class Greeter(string greeting, TextWriter output)
{
    public void Greet(string name) => output.WriteLine($"{greeting}, {name}!");
}

// Inline arrays: a fixed-size buffer struct usable like a Span, without unsafe.
[InlineArray(4)]
struct FourInts
{
    private int _element0;
}

static class CSharp12
{
    // ref readonly parameters: pass by reference, read-only, and the call site
    // makes it explicit.
    static double Norm(ref readonly (double X, double Y) p) => Sqrt(p.X * p.X + p.Y * p.Y);

    // [Experimental]: using this API is a compile-time error unless the
    // diagnostic is explicitly suppressed - for preview APIs in libraries.
    [Experimental("SHOW001")]
    static string PreviewApi() => "experimental API called";

    public static void Run()
    {
        Out.Section("C# 12");

        new Greeter("Hello", Console.Out).Greet("primary constructors");

        // Collection expressions: one syntax for arrays, lists, spans, sets...
        // and the spread element "..".
        int[] a = [1, 2, 3];
        List<int> b = [4, 5];
        Span<int> onStack = [7, 8, 9];
        HashSet<string> set = ["x", "y", "x"];
        int[] combined = [0, .. a, .. b, 6];
        List<int> empty = [];
        Console.WriteLine($"collection expressions: [{string.Join(",", combined)}], span len {onStack.Length}, set count {set.Count}, empty {empty.Count}");

        // Default values for lambda parameters (and params arrays in lambdas).
        var greet = (string name = "world") => $"hi {name}";
        Console.WriteLine($"default lambda parameter: {greet()} / {greet("you")}");

        // Alias any type (declared at the top: using Point3 = (double, double, double)).
        Point3 origin = (0, 0, 0);
        Point3 target = (3, 4, 12);
        Console.WriteLine($"type alias for tuple: distance = {Sqrt(Pow(target.X - origin.X, 2) + Pow(target.Y, 2) + Pow(target.Z, 2))}");

        var buf = new FourInts();
        for (int i = 0; i < 4; i++) buf[i] = i * 10;
        Span<int> asSpan = buf;
        Console.WriteLine($"inline array: [{string.Join(",", asSpan.ToArray())}]");

        var pt = (3.0, 4.0);
        Console.WriteLine($"ref readonly parameter: {Norm(in pt)}");

#pragma warning disable SHOW001
        Console.WriteLine($"[Experimental]: {PreviewApi()}");
#pragma warning restore SHOW001
    }
}

// ############################################################################
//  C# 13 (2024, .NET 9)
// ############################################################################

// Partial properties: declaration in one part, implementation in another
// (used by source generators, e.g. MVVM toolkits, GeneratedRegex).
partial class Thermostat
{
    public partial double Celsius { get; set; }
}
partial class Thermostat
{
    private double _celsius;
    public partial double Celsius
    {
        get => _celsius;
        set => _celsius = Math.Clamp(value, -50, 50);
    }
}

// Source-generated regular expressions (.NET 7+, relies on partial methods
// and properties): the regex is compiled to C# at build time.
static partial class Patterns
{
    [GeneratedRegex(@"(?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2})")]
    public static partial Regex IsoDate { get; }
}

static class CSharp13
{
    // params collections: "params" works with Span, ReadOnlySpan, List,
    // IEnumerable... params ReadOnlySpan<T> avoids allocating an array.
    static int Sum(params ReadOnlySpan<int> values)
    {
        int s = 0;
        foreach (var v in values) s += v;
        return s;
    }

    // OverloadResolutionPriority: library authors can steer overload choice.
    [OverloadResolutionPriority(1)]
    static string Pick(params ReadOnlySpan<string> items) => $"span overload ({items.Length})";
    static string Pick(params string[] items) => $"array overload ({items.Length})";

    // "allows ref struct" anti-constraint: generic code that accepts Span<T> & co.
    static int LengthOf<T>(T value, Func<T, int> measure) where T : allows ref struct => measure(value);

    // ref struct types can implement interfaces.
    ref struct SpanCounter : IDisposable
    {
        public int Disposed;
        public void Dispose() => Disposed++;
    }

    // ref locals / ref structs in iterators and async methods (not across yield/await).
    static IEnumerable<int> EvenIndexes(int[] data)
    {
        for (int i = 0; i < data.Length; i += 2)
        {
            {
                ReadOnlySpan<int> view = data;        // ref struct local inside an iterator
                if (view[i] < 0) continue;
            }
            yield return i;
        }
    }

    class Countdown
    {
        public int[] Slots { get; } = new int[5];
    }

    public static void Run()
    {
        Out.Section("C# 13");

        Console.WriteLine($"params ReadOnlySpan: {Sum(1, 2, 3, 4)}");
        Console.WriteLine($"OverloadResolutionPriority: {Pick("a", "b")}");

        // New System.Threading.Lock type + the lock statement uses its fast path.
        var gate = new Lock();
        int shared = 0;
        Parallel.For(0, 1000, _ => { lock (gate) { shared++; } });
        using (gate.EnterScope()) shared++;
        Console.WriteLine($"System.Threading.Lock: {shared}");

        // \e escape sequence = ESC (U+001B), handy for ANSI terminal colors.
        string esc = "\e[1mbold\e[0m";
        Console.WriteLine($"\\e escape: first char code = {(int)esc[0]}");

        // Implicit "from the end" index in object initializers.
        var cd = new Countdown { Slots = { [^1] = 0, [^2] = 1, [^3] = 2 } };
        Console.WriteLine($"^index in initializer: [{string.Join(",", cd.Slots)}]");

        Console.WriteLine($"allows ref struct: {LengthOf<ReadOnlySpan<char>>("span!".AsSpan(), s => s.Length)}");
        var counter = new SpanCounter();
        counter.Dispose();
        Console.WriteLine($"ref struct implementing interface: disposed {counter.Disposed}x");
        Console.WriteLine($"ref locals in iterator: [{string.Join(",", EvenIndexes([5, -1, -2, 7, 9]))}]");

        var t = new Thermostat { Celsius = 80 };
        Console.WriteLine($"partial property: clamped to {t.Celsius}");

        var match = Patterns.IsoDate.Match("Released 2024-11-12");
        Console.WriteLine($"[GeneratedRegex] partial property: year={match.Groups["year"].Value}");
    }
}

// ############################################################################
//  C# 14 (2025, .NET 10)
// ############################################################################

// Extension members: the new "extension(...)" block declares extension
// PROPERTIES, extension STATIC members and extension OPERATORS, in addition
// to classic extension methods. The receiver is declared once per block.
public record struct Temperature(double Celsius);

static class ModernExtensions
{
    extension(string text)
    {
        // Extension property
        public bool IsPalindrome
        {
            get
            {
                var letters = text.Where(char.IsLetterOrDigit).Select(char.ToLowerInvariant).ToArray();
                return letters.SequenceEqual(letters.Reverse());
            }
        }

        // Extension method declared inside the block
        public string Mirror() => text + new string(text.Reverse().ToArray());
    }

    // Static extension members: callable on the type itself (string.Separator).
    extension(string)
    {
        public static string Separator => new('-', 20);
    }

    // Generic extension block
    extension<T>(IEnumerable<T> source)
    {
        public bool IsEmpty => !source.Any();
    }

    // Extension operators
    extension(Temperature)
    {
        public static Temperature operator +(Temperature a, Temperature b) => new(a.Celsius + b.Celsius);
    }
}

// The "field" keyword: access the compiler-generated backing field from a
// property accessor - no need to declare a private field yourself.
class Account
{
    public string Owner
    {
        get;
        set => field = string.IsNullOrWhiteSpace(value) ? "unknown" : value.Trim();
    } = "unknown";

    public decimal Balance { get; private set; }

    // User-defined compound assignment operators: "+=" implemented as an
    // in-place instance operator (no new object allocated).
    public void operator +=(decimal amount) => Balance += amount;

    // User-defined instance increment operator.
    public void operator ++() => Balance += 1;
}

// Partial constructors and partial events (complements partial properties
// for source generators).
partial class Sensor
{
    public partial Sensor(string id);
    public partial event Action<double>? Reading;
}
partial class Sensor
{
    private Action<double>? _reading;
    public string Id { get; }
    public partial Sensor(string id) => Id = id;
    public partial event Action<double>? Reading
    {
        add => _reading += value;
        remove => _reading -= value;
    }
    public void Emit(double v) => _reading?.Invoke(v);
}

class Profile
{
    public string? Nickname { get; set; }
}

// Delegate with an out parameter for the lambda-modifier demo.
delegate bool TryParser<T>(string text, out T value);

static class CSharp14
{
    static int CountLong(ReadOnlySpan<string> words) => words.ToArray().Count(w => w.Length > 3);

    public static void Run()
    {
        Out.Section("C# 14");

        Console.WriteLine($"extension property: \"Never odd or even\".IsPalindrome = {"Never odd or even".IsPalindrome}");
        Console.WriteLine($"extension method in block: {"abc".Mirror()}");
        Console.WriteLine($"static extension property: {string.Separator}");
        Console.WriteLine($"generic extension property: {Array.Empty<int>().IsEmpty} / {new[] { 1 }.IsEmpty}");
        var t = new Temperature(20) + new Temperature(1.5);
        Console.WriteLine($"extension operator: {t.Celsius} C");

        var acc = new Account { Owner = "   Ada  " };
        acc += 100m;
        acc++;
        Console.WriteLine($"'field' keyword: owner='{acc.Owner}', compound assignment operator: balance={acc.Balance}");

        // Null-conditional assignment: assign only if the receiver is not null.
        Profile? present = new();
        Profile? missing = null;
        present?.Nickname = "neo";
        missing?.Nickname = "ignored";            // no NullReferenceException
        Console.WriteLine($"null-conditional assignment: {present?.Nickname}, missing is still {(missing is null ? "null" : "set")}");

        // nameof with unbound generic types.
        Console.WriteLine($"nameof(List<>) = {nameof(List<>)}, nameof(Dictionary<,>) = {nameof(Dictionary<,>)}");

        // Implicit span conversions: arrays convert to (ReadOnly)Span implicitly,
        // including for extension method receivers and generic inference.
        string[] words = ["tiny", "enormous", "ok", "huge"];
        Console.WriteLine($"implicit Span conversion: {CountLong(words)} long words");

        // Modifiers on simple lambda parameters (no types needed with out/ref/in).
        TryParser<int> parser = (text, out result) => int.TryParse(text, out result);
        Console.WriteLine($"lambda with 'out' modifier: {(parser("123", out var n) ? n : -1)}");

        var sensor = new Sensor("T-1");
        double last = 0;
        sensor.Reading += v => last = v;
        sensor.Emit(21.5);
        Console.WriteLine($"partial constructor/event: {sensor.Id} reported {last}");
    }
}

// ############################################################################
//  Modern .NET library highlights (.NET 5 - .NET 10)
// ############################################################################

// System.Text.Json with a source generator: serialization code is generated
// at compile time (fast, reflection-free, AOT friendly).
record WeatherReading(string City, double TempC, DateOnly Date);

[JsonSourceGenerationOptions(WriteIndented = false, PropertyNamingPolicy = JsonKnownNamingPolicy.CamelCase)]
[JsonSerializable(typeof(WeatherReading))]
partial class AppJsonContext : JsonSerializerContext { }

static class ModernLibrary
{
    public static async Task RunAsync()
    {
        Out.Section(".NET library highlights");

        // Span<T> / Memory<T>: slicing without copying.
        ReadOnlySpan<char> line = "id=42;name=widget";
        int sep = line.IndexOf(';');
        Console.WriteLine($"Span slicing: '{line[..sep]}' and '{line[(sep + 1)..]}'");

        // JSON with source generation.
        var reading = new WeatherReading("Munich", 7.5, new DateOnly(2025, 11, 11));
        string json = JsonSerializer.Serialize(reading, AppJsonContext.Default.WeatherReading);
        var back = JsonSerializer.Deserialize(json, AppJsonContext.Default.WeatherReading);
        Console.WriteLine($"System.Text.Json (source-gen): {json} -> round-trip equal: {back == reading}");

        // DateOnly / TimeOnly (.NET 6)
        var date = new DateOnly(2026, 11, 7);
        Console.WriteLine($"DateOnly: {date:yyyy-MM-dd} is a {date.DayOfWeek}, TimeOnly: {new TimeOnly(13, 45)}");

        // LINQ additions: Chunk/MinBy/MaxBy/DistinctBy (.NET 6), Order (.NET 7),
        // CountBy/AggregateBy/Index (.NET 9), LeftJoin and Shuffle (.NET 10).
        var people = new[] { ("Ann", 31, "dev"), ("Bob", 27, "ops"), ("Cid", 45, "dev"), ("Dee", 27, "qa") };
        Console.WriteLine($"Chunk(2): {string.Join(" | ", Enumerable.Range(1, 5).Chunk(2).Select(c => string.Join(",", c)))}");
        Console.WriteLine($"MinBy/MaxBy: {people.MinBy(p => p.Item2).Item1}/{people.MaxBy(p => p.Item2).Item1}, DistinctBy age: {people.DistinctBy(p => p.Item2).Count()}");
        Console.WriteLine($"Order: {string.Join(",", new[] { 3, 1, 2 }.Order())}");
        Console.WriteLine($"CountBy: {string.Join(", ", people.CountBy(p => p.Item3).Select(kv => $"{kv.Key}={kv.Value}"))}");
        Console.WriteLine($"AggregateBy: {string.Join(", ", people.AggregateBy(p => p.Item3, 0, (sum, p) => sum + p.Item2).Select(kv => $"{kv.Key}:{kv.Value}"))}");
        foreach (var (i, p) in people.Take(2).Index()) Console.Write($"#{i}={p.Item1} ");
        Console.WriteLine("<- Index()");
        var departments = new[] { ("dev", "Building A"), ("ops", "Building B") };
        var joined = people.LeftJoin(departments, p => p.Item3, d => d.Item1, (p, d) => $"{p.Item1}@{d.Item2 ?? "?"}");
        Console.WriteLine($"LeftJoin (.NET 10): {string.Join(", ", joined)}");
        Console.WriteLine($"Shuffle (.NET 10) keeps all items: {Enumerable.Range(1, 5).Shuffle().Order().SequenceEqual(Enumerable.Range(1, 5))}");

        // PriorityQueue (.NET 6)
        var pq = new PriorityQueue<string, int>();
        pq.Enqueue("low", 5); pq.Enqueue("urgent", 1); pq.Enqueue("normal", 3);
        Console.WriteLine($"PriorityQueue: first = {pq.Dequeue()}");

        // Frozen collections (.NET 8): immutable, optimized for read-heavy lookups.
        FrozenDictionary<string, int> httpCodes = new Dictionary<string, int> { ["OK"] = 200, ["NotFound"] = 404 }.ToFrozenDictionary();
        Console.WriteLine($"FrozenDictionary: NotFound = {httpCodes["NotFound"]}");

        // SearchValues (.NET 8): vectorized "IndexOfAny" for a fixed set.
        SearchValues<char> vowels = SearchValues.Create("aeiou");
        Console.WriteLine($"SearchValues: first vowel in 'rhythm and blues' at {"rhythm and blues".AsSpan().IndexOfAny(vowels)}");

        // Random.Shared, GetItems, Shuffle (.NET 6/8)
        var rng = new Random(42);
        int[] deck = Enumerable.Range(1, 6).ToArray();
        rng.Shuffle(deck);
        Console.WriteLine($"Random.Shuffle/GetItems: deck has {deck.Length}, picked {rng.GetItems(["red", "green"], 3).Length} items, Random.Shared ok: {Random.Shared.Next(1, 2) == 1}");

        // TimeProvider (.NET 8): abstract time for testable code.
        TimeProvider clock = TimeProvider.System;
        Console.WriteLine($"TimeProvider: utc year = {clock.GetUtcNow().Year}");

        // Task.WhenEach (.NET 9): process tasks in completion order.
        var tasks = new[] { Delayed(30, "slow"), Delayed(5, "fast"), Delayed(15, "medium") };
        await foreach (var done in Task.WhenEach(tasks)) Console.Write($"{await done} ");
        Console.WriteLine("<- Task.WhenEach completion order");

        // Channels: high-performance async producer/consumer queues.
        var channel = Channel.CreateBounded<int>(capacity: 2);
        var producer = Task.Run(async () =>
        {
            for (int i = 1; i <= 4; i++) await channel.Writer.WriteAsync(i);
            channel.Writer.Complete();
        });
        int total = 0;
        await foreach (var item in channel.Reader.ReadAllAsync()) total += item;
        await producer;
        Console.WriteLine($"System.Threading.Channels: consumed sum = {total}");

        // Parallel.ForEachAsync (.NET 6)
        int processed = 0;
        await Parallel.ForEachAsync(Enumerable.Range(1, 10), async (n, ct) =>
        {
            await Task.Delay(1, ct);
            Interlocked.Increment(ref processed);
        });
        Console.WriteLine($"Parallel.ForEachAsync: processed {processed}");

        // Guid v7 (.NET 9): time-ordered GUIDs, ideal as database keys.
        Guid g1 = Guid.CreateVersion7(), g2 = Guid.CreateVersion7();
        Console.WriteLine($"Guid.CreateVersion7: versions {g1.Version}/{g2.Version}, timestamp prefix {g1.ToString()[..8]}");

        // Base64Url (.NET 9) and Convert.ToHexString(Lower) (.NET 5/9)
        byte[] bytes = [0xFB, 0xFF, 0x01];
        Console.WriteLine($"Base64Url: {Base64Url.EncodeToString(bytes)}, hex: {Convert.ToHexStringLower(bytes)}");

        // Generic OrderedDictionary (.NET 9) and ReadOnlySet (.NET 9)
        var ordered = new OrderedDictionary<string, int> { ["z"] = 1, ["a"] = 2 };
        ordered.Insert(0, "first", 0);
        var ro = new ReadOnlySet<int>(new HashSet<int> { 1, 2 });
        Console.WriteLine($"OrderedDictionary: {string.Join(",", ordered.Keys)}, ReadOnlySet count {ro.Count}");

        // Throw helpers (.NET 6/8)
        try { ArgumentOutOfRangeException.ThrowIfNegative(-1); }
        catch (ArgumentOutOfRangeException ex) { Console.WriteLine($"ThrowIfNegative: {ex.ParamName}"); }

        // CollectionsMarshal: direct span access to a List's backing array.
        List<int> list = [1, 2, 3];
        foreach (ref int item in CollectionsMarshal.AsSpan(list)) item *= 10;
        Console.WriteLine($"CollectionsMarshal.AsSpan: [{string.Join(",", list)}]");

        // Half (.NET 5) and vectorized math via Vector<T> / generic math
        Half h = (Half)1.5f;
        var v = new Vector<float>(2f) * new Vector<float>(3f);
        Console.WriteLine($"Half: {h}, Vector<float>[0] = {v[0]} (lanes: {Vector<float>.Count})");

        // String.Create with a span-based writer (no intermediate buffers).
        string created = string.Create(5, 'x', (span, ch) => span.Fill(ch));
        Console.WriteLine($"string.Create: {created}");
    }

    static async Task<string> Delayed(int ms, string name)
    {
        await Task.Delay(ms);
        return name;
    }
}
