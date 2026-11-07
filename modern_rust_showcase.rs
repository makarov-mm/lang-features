//! ===========================================================================
//!  modern_rust_showcase.rs
//!
//!  A single-file tour of Rust from 1.0 (May 2015) to Rust 1.97 (2026),
//!  covering all three edition switches (2018, 2021, 2024). Every feature is
//!  annotated with the Rust version that stabilized it, how it works, and
//!  what it is used for.
//!
//!  Build & run (no Cargo project needed, no external crates):
//!      rustc --edition 2024 -O modern_rust_showcase.rs
//!      ./modern_rust_showcase          (modern_rust_showcase.exe on Windows)
//!
//!  Requires stable Rust 1.97 or newer.
//! ===========================================================================

// Inner attributes apply to the whole crate. `#![warn(...)]` raises lint
// levels; here we ask for warnings on missing `unsafe` blocks inside unsafe
// functions (already a warning by default in edition 2024).
#![warn(unsafe_op_in_unsafe_fn)]

use std::cell::{Cell, LazyCell, OnceCell, RefCell};
use std::collections::{BTreeMap, HashMap, VecDeque};
use std::error::Error;
use std::fmt;
use std::future::{Future, IntoFuture};
use std::num::{NonZero, Saturating};
use std::ops::ControlFlow;
use std::pin::{Pin, pin};
use std::rc::Rc;
use std::sync::atomic::{AtomicU32, AtomicUsize, Ordering};
use std::sync::mpsc;
use std::sync::{Arc, LazyLock, Mutex, OnceLock, RwLock};
use std::task::{Context, Poll, Wake, Waker};
use std::thread;

/// Prints a section header.
fn section(title: &str) {
    println!("\n=== {title} ===");
}

// ###########################################################################
//  Rust 1.0 (2015): the foundations
// ###########################################################################
mod foundations {
    use super::*;

    // Traits are Rust's interfaces. Generic code is constrained by traits
    // ("trait bounds") and is monomorphized - zero-cost at runtime.
    pub trait Shape {
        fn area(&self) -> f64;
        // Default method implementation.
        fn describe(&self) -> String {
            format!("shape with area {:.2}", self.area())
        }
    }

    #[derive(Debug, Clone, Copy, PartialEq)]
    pub struct Circle {
        pub r: f64,
    }
    impl Shape for Circle {
        fn area(&self) -> f64 {
            std::f64::consts::PI * self.r * self.r
        }
    }

    // Algebraic data types: enums whose variants carry data.
    #[derive(Debug)]
    pub enum Message {
        Quit,
        Move { x: i32, y: i32 },
        Write(String),
    }

    // Explicit lifetimes: the returned reference lives as long as both inputs.
    fn longest<'a>(a: &'a str, b: &'a str) -> &'a str {
        if a.len() >= b.len() { a } else { b }
    }

    pub fn demo() {
        section("Rust 1.0 foundations");

        // Ownership: every value has exactly one owner; assignment MOVES.
        let s1 = String::from("owned");
        let s2 = s1; // s1 is no longer usable - checked at compile time
        // Borrowing: many shared (&T) borrows OR one mutable (&mut T) borrow.
        let len = calc_len(&s2);
        println!("ownership/borrowing: '{}' has length {}", s2, len);

        // Pattern matching must be exhaustive.
        for msg in [
            Message::Move { x: 1, y: 2 },
            Message::Write("hi".into()),
            Message::Quit,
        ] {
            let text = match msg {
                Message::Quit => "quit".to_string(),
                Message::Move { x, y } => format!("move to ({x}, {y})"),
                Message::Write(t) => format!("write '{t}'"),
            };
            println!("enum + match: {text}");
        }

        // Option/Result instead of null and exceptions.
        let maybe: Option<i32> = "42".parse().ok();
        let parsed: Result<i32, _> = "x".parse::<i32>();
        println!("Option: {:?}, Result is_err: {}", maybe, parsed.is_err());

        // Closures and iterator adapters (lazy, fused into tight loops).
        let factor = 3;
        let v: Vec<i32> = (1..=10).filter(|n| n % 2 == 0).map(|n| n * factor).collect();
        println!("iterators + closures: {:?}", v);

        // Traits & generics
        let c = Circle { r: 1.5 };
        println!("trait default method: {}", c.describe());
        println!("lifetimes: longest = {}", longest("short", "much longer"));

        // Smart pointers: Box (heap), Rc (shared ownership, single thread),
        // RefCell (interior mutability with run-time borrow checking).
        let shared = Rc::new(RefCell::new(vec![1, 2]));
        let alias = Rc::clone(&shared);
        alias.borrow_mut().push(3);
        let boxed: Box<dyn Shape> = Box::new(c);
        println!(
            "Rc<RefCell>: {:?} (strong count {}), Box<dyn Shape> area {:.2}",
            shared.borrow(),
            Rc::strong_count(&shared),
            boxed.area()
        );

        // Fearless concurrency: Arc + Mutex, channels. The compiler rejects
        // data races through the Send/Sync marker traits.
        let counter = Arc::new(Mutex::new(0));
        let (tx, rx) = mpsc::channel();
        let handles: Vec<_> = (0..4)
            .map(|i| {
                let counter = Arc::clone(&counter);
                let tx = tx.clone();
                thread::spawn(move || {
                    *counter.lock().unwrap() += 1;
                    tx.send(i * i).unwrap();
                })
            })
            .collect();
        drop(tx);
        for h in handles {
            h.join().unwrap();
        }
        let received: i32 = rx.iter().sum();
        println!("threads: counter = {}, channel sum = {}", *counter.lock().unwrap(), received);
    }

    fn calc_len(s: &str) -> usize {
        s.len()
    }
}

// ###########################################################################
//  Rust 1.13 - 1.30 (2016 - 2018)
// ###########################################################################
mod early_evolution {
    use super::*;
    use std::convert::TryFrom;

    #[derive(Debug)]
    pub struct AppError(String);
    impl fmt::Display for AppError {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            write!(f, "AppError: {}", self.0)
        }
    }
    impl Error for AppError {}

    // `?` operator (1.13): on Err, return early (converting the error with
    // From::from); on Ok, unwrap the value. Replaced the old try!() macro.
    fn sum_parsed(a: &str, b: &str) -> Result<i64, Box<dyn Error>> {
        let x: i64 = a.trim().parse()?;
        let y: i64 = b.trim().parse()?;
        Ok(x + y)
    }

    // `?` also works on Option (1.22): None short-circuits.
    fn first_char_upper(s: &str) -> Option<char> {
        let c = s.chars().next()?;
        c.to_uppercase().next()
    }

    // impl Trait in return position (1.26): return an unnamed concrete type
    // (e.g. a closure or a long iterator chain) without boxing.
    fn evens(limit: u32) -> impl Iterator<Item = u32> {
        (0..limit).filter(|n| n % 2 == 0)
    }

    // impl Trait in argument position (1.26): sugar for a generic parameter.
    fn total(items: impl IntoIterator<Item = u32>) -> u32 {
        items.into_iter().sum()
    }

    // Associated constants (1.20).
    trait Unit {
        const SYMBOL: &'static str;
        const FACTOR: f64;
    }
    struct Kilometer;
    impl Unit for Kilometer {
        const SYMBOL: &'static str = "km";
        const FACTOR: f64 = 1000.0;
    }

    // Unions (1.19): C-compatible untagged unions; reading a field is unsafe.
    #[repr(C)]
    union IntOrFloat {
        i: u32,
        f: f32,
    }

    // #[must_use] on functions (1.27): ignoring the return value warns.
    #[must_use]
    fn checksum(data: &[u8]) -> u32 {
        data.iter().map(|&b| b as u32).sum()
    }

    // Procedural macros (custom #[derive] since 1.15, attribute and
    // function-like macros since 1.30) need their own proc-macro crate, so
    // they cannot live in this single file; #[derive(Debug, Default)] below
    // uses the built-in derives that work the same way.

    // Field init shorthand (1.17) + `Self` in struct literals.
    #[derive(Debug, Default)]
    struct Config {
        name: String,
        retries: u32,
    }
    impl Config {
        fn new(name: String, retries: u32) -> Self {
            Self { name, retries }
        }
    }

    pub fn demo() -> Result<(), Box<dyn Error>> {
        section("Rust 1.13 - 1.30");

        println!("? operator: {}", sum_parsed("40", " 2 ")?);
        match sum_parsed("4o", "2") {
            Ok(_) => {}
            Err(e) => println!("? propagated error: {e}"),
        }
        let custom: Result<(), Box<dyn Error>> = Err(Box::new(AppError("boom".into())));
        println!("custom Error type: {}", custom.unwrap_err());
        println!("? on Option: {:?} / {:?}", first_char_upper("rust"), first_char_upper(""));

        println!("impl Trait return: {:?}", evens(10).collect::<Vec<_>>());
        println!("impl Trait arg: {}", total(vec![1, 2, 3]));
        println!("associated consts: 1 {} = {} m", Kilometer::SYMBOL, Kilometer::FACTOR);

        let u = IntOrFloat { f: 1.0 };
        // SAFETY: both fields are 4 bytes and every bit pattern is a valid u32.
        let bits = unsafe { u.i };
        println!("union: bits of 1.0f32 = {bits:#x}");

        // loop can return a value via `break value` (1.19).
        let mut n = 1u64;
        let first_big = loop {
            n *= 3;
            if n > 1000 {
                break n;
            }
        };
        println!("break with value from loop: {first_big}");

        // 128-bit integers (1.26).
        let huge: u128 = u64::MAX as u128 * 3;
        println!("u128: {huge}");

        // Inclusive ranges `..=` in expressions and patterns (1.26).
        let grade = |score: u8| match score {
            90..=100 => 'A',
            75..=89 => 'B',
            _ => 'C',
        };
        println!("inclusive ranges: {} {} {}", grade(95), grade(80), grade(10));

        // Default binding modes, a.k.a. "match ergonomics" (1.26): matching a
        // reference with a non-reference pattern binds fields by reference.
        let pair = &(String::from("left"), 7);
        let (name, num) = pair; // name: &String, num: &i32
        println!("match ergonomics: {name} {num}");

        // `dyn Trait` syntax for trait objects (1.27).
        let shapes: Vec<Box<dyn foundations::Shape>> = vec![Box::new(foundations::Circle { r: 1.0 })];
        println!("dyn Trait: area = {:.3}", shapes[0].area());

        // Raw identifiers (1.30): use keywords as identifiers.
        let r#match = "keyword as name";
        println!("raw identifier: {}", r#match);

        // TryFrom / TryInto (1.34): fallible conversions.
        let small = u8::try_from(300i32);
        let ok: Result<u8, _> = 200i32.try_into();
        println!("TryFrom: {:?}, TryInto: {:?}", small.is_err(), ok);

        println!("#[must_use] checksum: {}", checksum(b"abc"));
        let cfg = Config::new("svc".into(), 3);
        let defaults = Config::default();
        println!("field init shorthand: {} x{}, Default: {:?}", cfg.name, cfg.retries, defaults);
        Ok(())
    }
}

// ###########################################################################
//  Edition 2018 (Rust 1.31) and releases up to 1.55
// ###########################################################################
mod edition_2018 {
    use super::*;

    // const fn (1.31; if/match/loop allowed since 1.46): evaluated at compile
    // time when used in a const context.
    const fn fib(n: u32) -> u64 {
        let (mut a, mut b) = (0u64, 1u64);
        let mut i = 0;
        while i < n {
            let t = a + b;
            a = b;
            b = t;
            i += 1;
        }
        a
    }
    const FIB_50: u64 = fib(50);

    // Const generics (1.51): types parameterized by constant values.
    #[derive(Debug)]
    struct Matrix<const R: usize, const C: usize> {
        data: [[i32; C]; R],
    }
    impl<const R: usize, const C: usize> Matrix<R, C> {
        fn transpose(&self) -> Matrix<C, R> {
            let mut out = [[0; R]; C];
            for (r, row) in self.data.iter().enumerate() {
                for (c, &v) in row.iter().enumerate() {
                    out[c][r] = v;
                }
            }
            Matrix { data: out }
        }
    }

    // #[non_exhaustive] (1.40): downstream crates must use a wildcard arm, so
    // adding variants later is not a breaking change.
    #[non_exhaustive]
    #[derive(Debug)]
    pub enum Protocol {
        Http,
        Https,
    }

    // #[track_caller] (1.46): panics report the *caller's* location.
    #[track_caller]
    fn checked_div(a: i32, b: i32) -> i32 {
        if b == 0 {
            panic!("division by zero");
        }
        a / b
    }

    // '_ anonymous lifetime (2018): "there is a lifetime here, infer it".
    struct Parser<'a> {
        input: &'a str,
    }
    impl Parser<'_> {
        fn words(&self) -> impl Iterator<Item = &str> {
            self.input.split_whitespace()
        }
    }

    pub fn demo() {
        section("Edition 2018 (1.31 - 1.55)");

        // Non-lexical lifetimes (NLL, 2018): borrows end at their last use,
        // not at the end of the scope, so this compiles.
        let mut scores = vec![10, 20];
        let first = &scores[0];
        println!("NLL: first = {first}");
        scores.push(30); // the shared borrow above is already dead
        println!("NLL: after push {:?}", scores);

        // dbg! (1.32): prints file:line, expression and value to stderr and
        // returns the value.
        let doubled = dbg!(scores.len() * 2);
        println!("dbg! returned {doubled}");

        println!("const fn: fib(50) computed at compile time = {FIB_50}");

        // std::iter::from_fn and successors (1.34)
        let mut count = 0;
        let counter = std::iter::from_fn(|| {
            count += 1;
            (count <= 3).then_some(count)
        });
        let powers: Vec<u32> = std::iter::successors(Some(1u32), |&x| x.checked_mul(10)).take(5).collect();
        println!("iter::from_fn: {:?}, successors: {:?}", counter.collect::<Vec<_>>(), powers);

        // std::any::type_name (1.38)
        println!("type_name: {}", std::any::type_name::<HashMap<String, Vec<u8>>>());

        // todo!() (1.40) marks unfinished code; here only referenced, not run.
        let _unfinished: fn() -> u32 = || todo!("implement later");

        // #[non_exhaustive]
        for p in [Protocol::Http, Protocol::Https] {
            let port = match p {
                Protocol::Http => 80,
                Protocol::Https => 443,
            };
            println!("#[non_exhaustive] enum: {p:?} -> port {port}");
        }

        // matches! macro (1.42)
        let c = 'x';
        println!("matches!: {}", matches!(c, 'a'..='z' | 'A'..='Z'));

        // Subslice patterns (1.42)
        let data: &[i32] = &[1, 2, 3, 4, 5];
        if let [first, middle @ .., last] = data {
            println!("subslice patterns: first={first} middle={middle:?} last={last}");
        }

        // Associated constants on primitives (1.43): u32::MAX instead of
        // std::u32::MAX.
        println!("u32::MAX = {}, f64::EPSILON = {:e}", u32::MAX, f64::EPSILON);

        // `as` casts from float to int saturate (1.45): no more UB.
        println!("saturating float cast: {} {}", 1e20f64 as i32, f64::NAN as u8);

        // #[track_caller]
        println!("#[track_caller]: {}", checked_div(10, 2));

        // bool::then (1.50)
        let even_label = (4 % 2 == 0).then(|| "even");
        println!("bool::then: {:?}", even_label);

        // Const generics
        let m = Matrix { data: [[1, 2, 3], [4, 5, 6]] };
        println!("const generics: {:?} -> {:?}", m.data, m.transpose().data);

        // Arrays implement IntoIterator by value (1.53) and traits for any
        // length (1.47).
        let words = ["alpha".to_string(), "beta".to_string()];
        for w in words {
            print!("{w} ");
        }
        println!("<- array by-value iteration");

        // Nested or-patterns (1.53)
        let pair = (1, 'b');
        if let (1 | 2, 'a' | 'b') = pair {
            println!("nested or-patterns matched");
        }

        // ControlFlow (1.55): early exit from try_for_each/try_fold.
        let found = (1..100).try_for_each(|n| {
            if n * n > 50 { ControlFlow::Break(n) } else { ControlFlow::Continue(()) }
        });
        println!("ControlFlow: first n with n^2 > 50 is {:?}", found);

        let parser = Parser { input: "anonymous lifetime demo" };
        println!("'_ lifetime: {} words", parser.words().count());
    }
}

// ###########################################################################
//  Minimal async runtime (used by the async demos)
// ###########################################################################
// Rust's async/await (1.39) compiles `async fn` into state machines that
// implement Future. The language provides no executor; normally you would use
// tokio or similar. This tiny `block_on` uses the Wake trait (1.51) to
// build a Waker that unparks the current thread.
mod mini_executor {
    use super::*;

    struct ThreadWaker(thread::Thread);
    impl Wake for ThreadWaker {
        fn wake(self: Arc<Self>) {
            self.0.unpark();
        }
    }

    pub fn block_on<F: Future>(fut: F) -> F::Output {
        // pin! (1.68) pins a value on the stack - required to poll a future.
        let mut fut = pin!(fut);
        let waker = Waker::from(Arc::new(ThreadWaker(thread::current())));
        let mut cx = Context::from_waker(&waker);
        loop {
            match fut.as_mut().poll(&mut cx) {
                Poll::Ready(v) => return v,
                Poll::Pending => thread::park(),
            }
        }
    }

    /// A future that is Pending once, then Ready - shows manual polling.
    pub struct YieldOnce(pub bool);
    impl Future for YieldOnce {
        type Output = ();
        fn poll(mut self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<()> {
            if self.0 {
                Poll::Ready(())
            } else {
                self.0 = true;
                cx.waker().wake_by_ref();
                Poll::Pending
            }
        }
    }
}

// ###########################################################################
//  Edition 2021 (Rust 1.56) and releases up to 1.84
// ###########################################################################
mod edition_2021 {
    use super::*;
    use mini_executor::{YieldOnce, block_on};

    // Generic associated types (GATs, 1.65): associated types with their own
    // generic parameters - e.g. a "lending iterator" whose items borrow from
    // the iterator itself.
    trait LendingIterator {
        type Item<'a>
        where
            Self: 'a;
        fn next(&mut self) -> Option<Self::Item<'_>>;
    }
    struct WindowsMut<'s> {
        data: &'s mut [i32],
        pos: usize,
    }
    impl<'s> LendingIterator for WindowsMut<'s> {
        type Item<'a>
            = &'a mut [i32]
        where
            Self: 'a;
        fn next(&mut self) -> Option<Self::Item<'_>> {
            let start = self.pos;
            if start + 2 > self.data.len() {
                return None;
            }
            self.pos += 1;
            Some(&mut self.data[start..start + 2])
        }
    }

    // Const generic defaults (1.59)
    struct Buffer<const N: usize = 4> {
        bytes: [u8; N],
    }

    // #[derive(Default)] on enums with #[default] (1.62)
    #[derive(Debug, Default, Clone, Copy, PartialEq)]
    enum Mode {
        Fast,
        #[default]
        Balanced,
        Precise,
    }

    // Explicit discriminants on enums with fields (1.66)
    #[repr(u8)]
    #[derive(Debug)]
    enum Packet {
        Ping = 1,
        Data(u16) = 2,
    }

    // async fn in traits & return-position impl Trait in traits (1.75)
    trait DataSource {
        async fn fetch(&self, id: u32) -> String;
        fn ids(&self) -> impl Iterator<Item = u32>;
    }
    struct MemorySource;
    impl DataSource for MemorySource {
        async fn fetch(&self, id: u32) -> String {
            YieldOnce(false).await;
            format!("record-{id}")
        }
        fn ids(&self) -> impl Iterator<Item = u32> {
            [7, 8].into_iter()
        }
    }

    // IntoFuture (1.64): `.await` works on any type implementing IntoFuture,
    // enabling builder patterns like `request.header(..).await`.
    struct Request {
        url: &'static str,
        retries: u32,
    }
    impl Request {
        fn retries(mut self, n: u32) -> Self {
            self.retries = n;
            self
        }
    }
    impl IntoFuture for Request {
        type Output = String;
        type IntoFuture = Pin<Box<dyn Future<Output = String>>>;
        fn into_future(self) -> Self::IntoFuture {
            Box::pin(async move { format!("GET {} (retries={})", self.url, self.retries) })
        }
    }

    // #[diagnostic::on_unimplemented] (1.78): custom compiler error message
    // when a trait bound is not satisfied.
    #[diagnostic::on_unimplemented(
        message = "`{Self}` cannot be stored in the registry",
        label = "implement `Storable` for this type"
    )]
    trait Storable {
        fn key(&self) -> String;
    }
    impl Storable for u32 {
        fn key(&self) -> String {
            format!("u32:{self}")
        }
    }

    // Associated type bounds (1.79): `impl Iterator<Item: Display>`.
    fn join_display(it: impl Iterator<Item: fmt::Display>) -> String {
        it.map(|x| x.to_string()).collect::<Vec<_>>().join("/")
    }

    // OnceLock (1.70) and LazyLock (1.80): thread-safe lazy statics without
    // lazy_static!/once_cell crates.
    static CONFIG: OnceLock<String> = OnceLock::new();
    static PRIMES: LazyLock<Vec<u32>> = LazyLock::new(|| {
        (2..50).filter(|n| (2..*n).all(|d| n % d != 0)).collect()
    });
    // const Mutex::new (1.63): Mutex in a static without lazy init.
    static LOG: Mutex<Vec<&'static str>> = Mutex::new(Vec::new());

    // offset_of! (1.77)
    #[repr(C)]
    struct Header {
        tag: u8,
        len: u32,
    }

    // Safe extern items (1.82): inside `unsafe extern` blocks, items marked
    // `safe` can be called without an unsafe block. Edition 2024 requires the
    // `unsafe` keyword on extern blocks because declaring foreign functions
    // with wrong signatures is unsound.
    unsafe extern "C" {
        safe fn abs(x: i32) -> i32;
        fn strlen(s: *const std::ffi::c_char) -> usize;
    }

    // Unsafe attributes (1.82): attributes that can cause UB must be written
    // as #[unsafe(...)]. no_mangle exports the symbol with its plain name.
    #[unsafe(no_mangle)]
    pub extern "C" fn showcase_exported_add(a: i32, b: i32) -> i32 {
        a + b
    }

    // Precise capturing `use<..>` (1.82): state exactly which generic
    // parameters/lifetimes a returned impl Trait may capture.
    fn lengths<'a>(words: &'a [String], _scratch: &mut Vec<u8>) -> impl Iterator<Item = usize> + use<'a> {
        words.iter().map(|w| w.len())
    }

    // Lint expectations (1.81): #[expect] is like #[allow] but warns if the
    // lint is NOT triggered - so stale suppressions get cleaned up.
    #[expect(dead_code, reason = "demonstrates #[expect]")]
    fn never_called() {}

    pub fn demo() {
        section("Edition 2021 (1.56 - 1.84)");

        // Disjoint closure captures (2021): closures capture only the fields
        // they use, not the whole struct.
        let mut point = (1, 2);
        let mut inc_x = || point.0 += 1;
        let y_ref = &point.1; // allowed: the closure only captures point.0
        inc_x();
        println!("disjoint captures: y = {y_ref}, x = {}", point.0);

        // Bindings after @ (1.56)
        if let whole @ Some(inner @ 1..=9) = Some(5) {
            println!("binding @ with sub-bindings: {whole:?} {inner}");
        }

        // panic! in const contexts (1.57): compile-time assertions.
        const _: () = assert!(std::mem::size_of::<u64>() == 8);

        // Iterator::map_while (1.57)
        let prefix: Vec<u32> = ["1", "2", "x", "4"].iter().map_while(|s| s.parse().ok()).collect();
        println!("map_while: {prefix:?}");

        // Captured identifiers in format strings (1.58)
        let user = "ferris";
        let width = 10;
        println!("format capture: [{user:>width$}]");

        // Destructuring assignment (1.59)
        let (mut a, mut b) = (1, 2);
        (a, b) = (b, a);
        [a, _] = [a * 10, 0];
        println!("destructuring assignment: a={a} b={b}");

        let buf: Buffer = Buffer { bytes: [0; 4] };
        println!("const generic default: N = {}", buf.bytes.len());

        // thread::available_parallelism (1.59)
        let cores = thread::available_parallelism().map(|n| n.get()).unwrap_or(1);
        println!("available_parallelism >= 1: {}", cores >= 1);

        println!("derive(Default) on enum: {:?}, all: {:?}", Mode::default(), [Mode::Fast, Mode::Precise]);
        println!("bool::then_some (1.62): {:?}", (cores > 0).then_some("has cores"));

        // Scoped threads (1.63): threads that may borrow local data, because
        // the scope guarantees they are joined before returning.
        let numbers = vec![1, 2, 3, 4, 5, 6];
        let (left, right) = numbers.split_at(3);
        let (sl, sr) = thread::scope(|s| {
            let h1 = s.spawn(|| left.iter().sum::<i32>());
            let h2 = s.spawn(|| right.iter().sum::<i32>());
            (h1.join().unwrap(), h2.join().unwrap())
        });
        println!("scoped threads: {sl} + {sr}");

        // std::array::from_fn (1.63)
        let squares: [u32; 5] = std::array::from_fn(|i| (i * i) as u32);
        println!("array::from_fn: {squares:?}");

        LOG.lock().unwrap().push("const Mutex::new");
        println!("static Mutex: {:?}", LOG.lock().unwrap());

        // let-else (1.65): bind or diverge.
        let input = "key=value";
        let Some((key, value)) = input.split_once('=') else {
            panic!("malformed input");
        };
        println!("let-else: {key} -> {value}");

        // Labeled block break (1.65)
        let category = 'block: {
            if input.is_empty() {
                break 'block "empty";
            }
            if input.contains('=') {
                break 'block "assignment";
            }
            "other"
        };
        println!("labeled block break: {category}");

        // GATs: lending iterator
        let mut data = [1, 2, 3, 4];
        let mut windows = WindowsMut { data: &mut data, pos: 0 };
        while let Some(w) = windows.next() {
            w[1] += w[0];
        }
        println!("GAT lending iterator (prefix sums): {:?}", data);

        for pkt in [Packet::Ping, Packet::Data(512)] {
            let payload = if let Packet::Data(len) = pkt { len } else { 0 };
            println!("enum discriminants with fields: {pkt:?} payload {payload}");
        }

        // std::hint::black_box (1.66): hide a value from the optimizer (benchmarks).
        let bb = std::hint::black_box(41) + 1;
        // ilog2/ilog10 and checked_ilog (1.67)
        println!("black_box: {bb}, ilog2(1024)={}, ilog10(12345)={}", 1024u32.ilog2(), 12345u32.ilog10());

        // OnceCell (1.70) - single-threaded write-once cell.
        let cell = OnceCell::new();
        let v = cell.get_or_init(|| "initialised once");
        println!("OnceCell: {v}");
        CONFIG.set("production".to_string()).ok();
        println!("OnceLock: {:?}, LazyLock primes: {:?}", CONFIG.get(), &PRIMES[..5]);

        // IsTerminal (1.70): is stdout a TTY?
        use std::io::IsTerminal;
        println!("IsTerminal: stdout is terminal = {}", std::io::stdout().is_terminal());

        // div_ceil (1.73), abs_diff (1.60), is_multiple_of (1.87)
        println!("div_ceil(7,2)={} abs_diff(3,10)={} 12.is_multiple_of(4)={}",
                 7u32.div_ceil(2), 3u32.abs_diff(10), 12u32.is_multiple_of(4));

        // Saturating<T> wrapper (1.74)
        let mut hp = Saturating(250u8);
        hp += 10;
        println!("Saturating<u8>: {}", hp);

        // async fn in traits + RPITIT + a hand-rolled executor
        let src = MemorySource;
        let records = block_on(async {
            let mut out = Vec::new();
            for id in src.ids() {
                out.push(src.fetch(id).await);
            }
            out
        });
        println!("async fn in trait: {records:?}");
        println!("IntoFuture builder: {}", block_on(async { Request { url: "/api", retries: 0 }.retries(3).await }));

        // std::future::poll_fn (1.64): build a future from a closure.
        let mut polled = 0;
        let value = block_on(std::future::poll_fn(|cx| {
            polled += 1;
            if polled < 3 {
                cx.waker().wake_by_ref();
                Poll::Pending
            } else {
                Poll::Ready(polled)
            }
        }));
        println!("poll_fn: ready after {value} polls");

        // Option::as_slice (1.75), Option::inspect / Arc::unwrap_or_clone (1.76)
        let some = Some(5);
        let _ = some.inspect(|v| println!("Option::inspect saw {v}, as_slice = {:?}", some.as_slice()));
        let arc = Arc::new(String::from("unique"));
        let owned: String = Arc::unwrap_or_clone(arc);
        println!("Arc::unwrap_or_clone: {owned}");

        // C-string literals (1.77): c"..." is a &CStr with a trailing NUL.
        let cstr = c"hello from C";
        // SAFETY: cstr is a valid NUL-terminated string.
        let n = unsafe { strlen(cstr.as_ptr()) };
        println!("c\"\" literal: {:?} strlen = {n}, safe extern abs(-9) = {}", cstr, abs(-9));
        println!("#[unsafe(no_mangle)] fn: {}", showcase_exported_add(2, 3));

        println!("offset_of!(Header, len) = {}", std::mem::offset_of!(Header, len));

        // slice::chunk_by (1.77) and array::each_ref (1.77)
        let runs = [1, 1, 2, 3, 3, 3, 1];
        let groups: Vec<&[i32]> = runs.chunk_by(|a, b| a == b).collect();
        let names = [String::from("a"), String::from("b")];
        let refs: [&String; 2] = names.each_ref();
        println!("chunk_by: {groups:?}, each_ref: {refs:?}");

        println!("#[diagnostic::on_unimplemented] trait: {}", 7u32.key());

        // Inline const blocks (1.79): force compile-time evaluation inline.
        let table = [const { Vec::<u8>::new() }; 3]; // Vec is not Copy - const block makes it work
        println!("inline const: {} empty vecs", table.len());

        println!("associated type bounds: {}", join_display([1.5, 2.5].into_iter()));

        // NonZero<T> generic type (1.79): Option<NonZero<u32>> is 4 bytes.
        let nz = NonZero::new(42u32).unwrap();
        println!("NonZero<u32>: {nz}, size_of Option<NonZero<u32>> = {}", std::mem::size_of::<Option<NonZero<u32>>>());

        // LazyCell (1.80)
        let lazy = LazyCell::new(|| {
            println!("  (LazyCell initializer runs now)");
            99
        });
        println!("LazyCell value: {}", *lazy);

        // Exclusive range patterns (1.80)
        let temp = 15;
        let feel = match temp {
            ..0 => "freezing",
            0..20 => "cool",
            20.. => "warm",
        };
        println!("exclusive range patterns: {feel}");

        // Option::take_if, slice::split_at_checked (1.80)
        let mut slot = Some(10);
        let taken = slot.take_if(|v| *v > 5);
        println!("take_if: {taken:?} -> {slot:?}, split_at_checked(10): {:?}", [1, 2].split_at_checked(10));

        // core::error::Error (1.81): the Error trait is available in no_std.
        fn takes_core_error(e: &dyn core::error::Error) -> String {
            e.to_string()
        }
        println!("core::error::Error: {}", takes_core_error(&std::fmt::Error));

        // &raw const / &raw mut (1.82): create raw pointers without creating
        // an intermediate reference (sound for packed/unaligned fields).
        #[repr(C, packed)]
        struct Packed {
            a: u8,
            b: u32,
        }
        let pk = Packed { a: 1, b: 0xDEAD_BEEF };
        let pb = &raw const pk.b;
        // SAFETY: pointer is valid; read_unaligned handles the misalignment.
        let b_val = unsafe { pb.read_unaligned() };
        println!("&raw const on packed field: {b_val:#x} (a = {})", pk.a);

        let words = vec!["one".to_string(), "three".to_string()];
        let mut scratch = Vec::new();
        println!("use<'a> precise capturing: {:?}", lengths(&words, &mut scratch).collect::<Vec<_>>());

        // Option::is_none_or, is_sorted, iter::repeat_n (1.82)
        println!("is_none_or: {}, is_sorted: {}, repeat_n: {:?}",
                 Some(3).is_none_or(|x| x > 1), [1, 2, 2, 5].is_sorted(),
                 std::iter::repeat_n("ab", 3).collect::<String>());

        // Patterns on empty types may be omitted (1.82): Result<T, Infallible>
        // is irrefutably Ok.
        let always_ok: Result<u8, std::convert::Infallible> = Ok(8);
        let Ok(eight) = always_ok;
        println!("empty-type patterns omitted: {eight}");

        // &mut in const fn and const items (1.83)
        const fn bump(x: &mut u32) {
            *x += 1;
        }
        const BUMPED: u32 = {
            let mut v = 41;
            bump(&mut v);
            v
        };
        let mut cache: Option<Vec<u8>> = None;
        cache.get_or_insert_default().push(1); // Option::get_or_insert_default (1.83)
        println!("const &mut: {BUMPED}, get_or_insert_default: {cache:?}");

        // isqrt (1.84) and strict provenance APIs (1.84)
        let arr = [10u16, 20, 30];
        let p = arr.as_ptr();
        let addr = p.addr(); // address without exposing provenance
        let p2 = p.with_addr(addr + std::mem::size_of::<u16>());
        // SAFETY: p2 stays within the array and keeps p's provenance.
        println!("isqrt(99)={}, strict provenance read: {}", 99u32.isqrt(), unsafe { *p2 });

        // Collect into a tuple of collections (1.79): unzip-like partitioning.
        let (evens, odds): (Vec<i32>, Vec<i32>) = (1..=6).partition(|n| n % 2 == 0);
        let (keys, vals): (Vec<char>, Vec<u32>) = [('a', 1), ('b', 2)].into_iter().collect();
        println!("partition: {evens:?}/{odds:?}, tuple collect: {keys:?}/{vals:?}");
    }
}

// ###########################################################################
//  Edition 2024 (Rust 1.85) and releases up to 1.97 (2025 - 2026)
// ###########################################################################
mod edition_2024 {
    use super::*;
    use mini_executor::block_on;

    // Trait upcasting (1.86): &dyn Sub can be coerced to &dyn Super.
    trait Named {
        fn name(&self) -> String;
    }
    trait Greeter: Named {
        fn greet(&self) -> String {
            format!("Hello from {}", self.name())
        }
    }
    struct Bot;
    impl Named for Bot {
        fn name(&self) -> String {
            "bot".into()
        }
    }
    impl Greeter for Bot {}

    // #[diagnostic::do_not_recommend] (1.85): hide an impl from error hints.
    trait Pretty {
        fn pretty(&self) -> String;
    }
    #[diagnostic::do_not_recommend]
    impl<T: fmt::Debug> Pretty for Vec<T> {
        fn pretty(&self) -> String {
            format!("<{self:?}>")
        }
    }

    // Explicit 128-bit enum representation (1.89)
    #[repr(u128)]
    #[derive(Debug, Clone, Copy)]
    enum Huge {
        Max = u128::MAX,
    }

    // Edition 2024 RPIT rule: `impl Trait` in return position captures all
    // in-scope generic parameters and lifetimes by default (use `use<..>` to
    // restrict). Precise capturing in trait methods arrived in 1.87.
    trait Source {
        fn items(&self) -> impl Iterator<Item = u32> + use<Self>;
    }
    struct Fixed;
    impl Source for Fixed {
        fn items(&self) -> impl Iterator<Item = u32> + use<> {
            [3, 1, 2].into_iter()
        }
    }

    pub fn demo() -> Result<(), Box<dyn Error>> {
        section("Edition 2024 (1.85 - 1.97)");

        // Async closures (1.85): `async |x| { .. }` closures that can borrow
        // from their captured environment across await points, with the new
        // AsyncFn / AsyncFnMut / AsyncFnOnce traits.
        let prefix = String::from("item");
        let make_label = async |n: u32| -> String {
            mini_executor::YieldOnce(false).await;
            format!("{prefix}-{n}")
        };
        async fn apply_all(f: impl AsyncFn(u32) -> String) -> Vec<String> {
            let mut v = Vec::new();
            for i in 1..=3 {
                v.push(f(i).await);
            }
            v
        }
        println!("async closures + AsyncFn: {:?}", block_on(apply_all(make_label)));

        // midpoint (1.85) and ptr::fn_addr_eq (1.85)
        fn f() {}
        println!("midpoint: {} {}, fn_addr_eq: {}", 1.0f64.midpoint(4.0), 3u32.midpoint(8),
                 std::ptr::fn_addr_eq(f as fn(), f as fn()));
        println!("do_not_recommend impl still usable: {}", vec![1, 2].pretty());

        // Trait upcasting
        let g: &dyn Greeter = &Bot;
        let n: &dyn Named = g; // upcast to the supertrait object
        println!("trait upcasting: {} / {}", g.greet(), n.name());

        // get_disjoint_mut (1.86): several &mut into one slice/map at once.
        let mut arr = [1, 2, 3, 4];
        if let Ok([a, b]) = arr.get_disjoint_mut([0, 3]) {
            std::mem::swap(a, b);
        }
        // Vec::pop_if (1.86)
        let mut stack = vec![1, 2, 10];
        let popped = stack.pop_if(|x| *x > 5);
        println!("get_disjoint_mut swap: {arr:?}, pop_if: {popped:?} -> {stack:?}");

        // Precise capturing in traits (1.87)
        println!("use<> in trait impl: {:?}", Fixed.items().collect::<Vec<_>>());

        // Anonymous pipes (1.87), Vec::extract_if (1.87), cast_signed & unbounded_shl (1.87)
        use std::io::{Read, Write};
        let (mut reader, mut writer) = std::io::pipe()?;
        writer.write_all(b"through a pipe")?;
        drop(writer);
        let mut piped = String::new();
        reader.read_to_string(&mut piped)?;
        let mut nums: Vec<i32> = (1..=8).collect();
        let evens: Vec<i32> = nums.extract_if(.., |x| *x % 2 == 0).collect();
        println!("io::pipe: '{piped}', extract_if: {evens:?} left {nums:?}");
        println!("cast_signed(255u8)={} unbounded_shl(1u32, 40)={}", 255u8.cast_signed(), 1u32.unbounded_shl(40));

        // let chains (1.88, edition 2024 only): `&&` between let-patterns
        // and boolean conditions in if / while.
        let cfg: HashMap<&str, &str> = HashMap::from([("port", "8080"), ("host", "localhost")]);
        if let Some(port) = cfg.get("port")
            && let Ok(port) = port.parse::<u16>()
            && port > 1024
        {
            println!("let chains: unprivileged port {port}");
        }

        // cfg(true)/cfg(false) boolean literals (1.88)
        #[cfg(true)]
        println!("#[cfg(true)] item compiled in");

        // Cell::update (1.88), slice::as_chunks (1.88), HashMap::extract_if (1.88),
        // hint::select_unpredictable (1.88)
        let cell = Cell::new(5);
        cell.update(|x| x * 3);
        let (chunks, rest) = [1, 2, 3, 4, 5].as_chunks::<2>();
        let mut inventory: HashMap<&str, i32> = HashMap::from([("a", 0), ("b", 4), ("c", 0)]);
        let mut empty: Vec<&str> = inventory.extract_if(|_, qty| *qty == 0).map(|(k, _)| k).collect();
        empty.sort();
        println!("Cell::update: {}, as_chunks: {chunks:?} rest {rest:?}, extract_if removed {empty:?}",
                 cell.get());
        println!("select_unpredictable: {}", std::hint::select_unpredictable(cell.get() > 10, "big", "small"));

        // Inferred const generic arguments with `_` (1.89)
        let inferred: [u8; _] = [1, 2, 3, 4];
        println!("[u8; _] inferred length = {}", inferred.len());

        // File locking (1.89) and Result::flatten (1.89)
        let path = std::env::temp_dir().join("modern_rust_showcase.lock");
        let file = std::fs::File::create(&path)?;
        file.lock()?; // exclusive advisory lock
        file.unlock()?;
        drop(file);
        std::fs::remove_file(&path)?;
        let nested: Result<Result<u8, String>, String> = Ok(Ok(1));
        println!("File::lock ok, Result::flatten: {:?}, repr(u128): {:?} = {}",
                 nested.flatten(), Huge::Max, Huge::Max as u128);

        // Duration::from_mins, strict_* arithmetic, Path::file_prefix,
        // str::floor_char_boundary, iter::chain function, BTreeMap::extract_if (1.91)
        let d = std::time::Duration::from_mins(2);
        let strict = 200u8.strict_add(55); // panics on overflow, even in release
        let prefix = std::path::Path::new("archive.tar.gz").file_prefix();
        let text = "na\u{ef}ve"; // "naive" with i-diaeresis: 2 bytes in UTF-8
        let cut = text.floor_char_boundary(3); // byte 3 is inside that char -> 2
        let chained: Vec<i32> = std::iter::chain([1, 2], [3]).collect();
        let mut bt: BTreeMap<i32, &str> = BTreeMap::from([(1, "a"), (2, "b"), (3, "c")]);
        let big: Vec<(i32, &str)> = bt.extract_if(2.., |_, _| true).collect();
        println!("from_mins: {d:?}, strict_add: {strict}, file_prefix: {prefix:?}, floor_char_boundary: '{}'",
                 &text[..cut]);
        println!("iter::chain: {chained:?}, BTreeMap::extract_if: {big:?} left {bt:?}");

        // RwLockWriteGuard::downgrade (1.92): turn a write lock into a read lock
        // atomically, without letting another writer in.
        let lock = RwLock::new(vec![1]);
        let mut w = lock.write().unwrap();
        w.push(2);
        let r = std::sync::RwLockWriteGuard::downgrade(w);
        println!("RwLock downgrade: {:?}", *r);
        drop(r);
        // NonZero::div_ceil (1.92)
        let nz = NonZero::new(7u32).unwrap();
        println!("NonZero::div_ceil: {}", nz.div_ceil(NonZero::new(2).unwrap()));

        // fmt::from_fn (1.93): implement Display/Debug ad hoc from a closure.
        let items = [1, 2, 3];
        let listing = fmt::from_fn(|f| {
            for (i, x) in items.iter().enumerate() {
                if i > 0 {
                    f.write_str(" -> ")?;
                }
                write!(f, "#{x}")?;
            }
            Ok(())
        });
        println!("fmt::from_fn: {listing}");
        // slice::as_array (1.93) and VecDeque::pop_front_if (1.93)
        let slice: &[i32] = &[9, 8, 7];
        let as_arr: Option<&[i32; 3]> = slice.as_array();
        let mut dq: VecDeque<i32> = VecDeque::from([1, 5, 9]);
        let front = dq.pop_front_if(|x| *x < 3);
        println!("as_array: {as_arr:?}, VecDeque::pop_front_if: {front:?}");

        // slice::array_windows (1.94), LazyLock::get (1.94),
        // Peekable::next_if_map (1.94), new math constants (1.94)
        let series = [1, 4, 9, 16];
        let deltas: Vec<i32> = series.array_windows().map(|[a, b]| b - a).collect();
        static LAZY: LazyLock<u32> = LazyLock::new(|| 7);
        let before = LazyLock::get(&LAZY).copied();
        let _ = *LAZY;
        let mut tokens = ["12", "x"].into_iter().peekable();
        let num = tokens.next_if_map(|t| t.parse::<i32>().map_err(|_| t));
        println!("array_windows deltas: {deltas:?}, LazyLock::get before/after: {before:?}/{:?}",
                 LazyLock::get(&LAZY));
        println!("next_if_map: {num:?}, golden ratio = {:.6}, Euler gamma = {:.6}",
                 std::f64::consts::GOLDEN_RATIO, std::f64::consts::EULER_GAMMA);

        // if-let guards in match arms (1.95)
        for s in ["10", "abc", ""] {
            let kind = match s {
                "" => "empty".to_string(),
                text if let Ok(n) = text.parse::<i32>() => format!("number {n}"),
                other => format!("word '{other}'"),
            };
            println!("if-let guard: {kind}");
        }

        // cfg_select! (1.95): a `match` over cfg predicates (replaces the
        // cfg-if crate).
        let platform = cfg_select! {
            windows => { "windows" }
            target_os = "linux" => { "linux" }
            target_os = "macos" => { "macos" }
            _ => { "other" }
        };
        println!("cfg_select!: compiled for {platform}");

        // Atomic update (1.95): read-modify-write with a closure.
        let hits = AtomicU32::new(10);
        let prev = hits.update(Ordering::SeqCst, Ordering::SeqCst, |x| x * 2);
        // Vec::push_mut (1.95): push and get a &mut to the new element.
        let mut list = vec![1];
        *list.push_mut(10) += 5;
        // bool::try_from(int) (1.95); hint::cold_path (1.95) marks unlikely paths.
        let flag = bool::try_from(1u8)?;
        if !flag {
            std::hint::cold_path();
        }
        println!("Atomic::update prev {prev} now {}, push_mut: {list:?}, bool::try_from(1u8) = {flag}",
                 hits.load(Ordering::SeqCst));

        // assert_matches! (1.96) and the new core::range types (1.95/1.96):
        // Copy-able range types that implement IntoIterator instead of Iterator.
        let parsed: Result<u16, _> = "443".parse::<u16>();
        std::assert_matches!(parsed, Ok(1..=1023));
        let r = core::range::Range::from(1..5);
        let r_copy = r; // Copy! (the old std::ops::Range is not Copy)
        println!("assert_matches! passed, core::range::Range {r:?} sum = {}", r_copy.into_iter().sum::<i32>());

        // bit_width, highest_one, isolate_lowest_one (1.97)
        let x = 0b0101_1000u32;
        println!("bit_width={} highest_one={:?} isolate_lowest_one={:#b}",
                 x.bit_width(), x.highest_one(), x.isolate_lowest_one());

        // Edition 2024 also: `gen` is a reserved keyword (for future
        // generators), `static mut` references are denied, `unsafe extern`
        // is mandatory, and tail-expression temporaries drop earlier.
        let gen_counter = AtomicUsize::new(0);
        gen_counter.fetch_add(1, Ordering::Relaxed);
        println!("edition 2024 rules respected: {}", gen_counter.load(Ordering::Relaxed));
        Ok(())
    }
}

// `main` may return Result (1.26): an Err is printed and the exit code is 1.
fn main() -> Result<(), Box<dyn Error>> {
    println!("Modern Rust feature showcase (1.0 .. 1.97, editions 2015/2018/2021/2024)");
    foundations::demo();
    early_evolution::demo()?;
    edition_2018::demo();
    edition_2021::demo();
    edition_2024::demo()?;
    println!("\nDone.");
    Ok(())
}
