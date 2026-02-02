# Go Structs vs C++ Classes: Performance Benchmarks

Benchmarks to substantiate claims in the blog post:

**["Go Structs Are Not C++ Classes: 7 Hardware-Level Differences"](https://blog.blackwell-systems.com/posts/go-structs-not-cpp-classes/)**

These are not "Go vs C++" speed contests.
They isolate specific CPU-level costs that show up when a design becomes:

- **Contiguous values + direct calls** (cache-friendly, predictable branches)
vs
- **Pointer graphs + indirect calls** (pointer chasing, indirect branches, GC/allocator pressure)

The key question is not *what either language can express*, but what their **common patterns** make easy.

---

## Benchmarks

### 1) Pointer Chasing vs Contiguous Memory

**What this demonstrates (precisely):**

In **heterogeneous polymorphic C++ designs** (multiple derived types stored behind a base interface),
you typically end up storing **pointers** in the container (e.g. `vector<Base*>` / `vector<unique_ptr<Base>>`)
because storing mixed derived objects inline causes **object slicing** and sizes differ across types.

Go does the same thing **only when you opt into it** (pointers / interface-heavy data structures).
Otherwise, Go's "slice of concrete values" pattern is the default.

**This benchmark isolates the cache locality cost of:**

- Contiguous array of values
- Array of pointers to separately-allocated objects (pointer chasing)

It does *not* claim C++ cannot store values contiguously.
It claims that classic inheritance-based polymorphism commonly pushes you toward pointer graphs.

#### Compile & Run (C++)

```bash
cd cpp
g++ -O3 -std=c++17 -o pointer_chasing pointer_chasing.cpp
./pointer_chasing
```

#### Run (Go)

```bash
cd go
go run pointer_chasing.go
```

---

### 2) Virtual Dispatch vs Static Dispatch (dispatch cost isolated)

**What this demonstrates:**

This benchmark attempts to measure **dispatch overhead** separately from memory layout.
Both languages pay similar costs for **indirect calls** (vtable / itab method lookup + indirect branch),
and both can optimize **direct calls** aggressively (inlining, constant propagation).

**Important constraint:**
Real-world C++ inheritance hierarchies often pay *both*:

* pointer chasing (benchmark 1) **and**
* vtable dispatch (this benchmark)

This benchmark focuses on the dispatch side and tries to avoid confounding it with pointer chasing.

**⚠️ Benchmarking caveat (compiler):**
Compilers can sometimes **devirtualize** virtual calls if the dynamic type is provably known,
turning them into direct calls. The benchmark code is written to prevent that.
Avoid compiling with LTO, and prefer running release builds only.

#### Compile & Run (C++)

```bash
cd cpp
g++ -O3 -std=c++17 -o virtual_dispatch virtual_dispatch.cpp
./virtual_dispatch
```

#### Run (Go)

```bash
cd go
go run virtual_dispatch.go
```

---

### 3) Heap vs Value Storage (allocation + storage pattern)

**What this demonstrates:**

Allocation overhead is rarely "just malloc vs stack."
Real programs allocate and then **store** objects in collections, which affects:

* cache locality
* GC / allocator pressure
* pointer graph scanning costs (Go)
* fragmentation / allocator metadata costs (C++)

This benchmark compares:

* Pointer-heavy storage (heap allocations + storing pointers)
* Value storage (contiguous storage in a single backing allocation)

#### Compile & Run (C++)

```bash
cd cpp
g++ -O3 -std=c++17 -o allocation allocation.cpp
./allocation
```

#### Run (Go)

```bash
cd go
go run allocation.go
```

To inspect escape analysis decisions:

```bash
go run -gcflags="-m" allocation.go 2>&1 | grep escape
```

---

## Running All Benchmarks

```bash
# C++ benchmarks
cd cpp
g++ -O3 -std=c++17 -o pointer_chasing pointer_chasing.cpp
g++ -O3 -std=c++17 -o virtual_dispatch virtual_dispatch.cpp
g++ -O3 -std=c++17 -o allocation allocation.cpp

echo "=== C++ Pointer Chasing ==="
./pointer_chasing
echo ""
echo "=== C++ Virtual Dispatch ==="
./virtual_dispatch
echo ""
echo "=== C++ Allocation ==="
./allocation

# Go benchmarks
cd ../go
echo ""
echo "=== Go Pointer Chasing ==="
go run pointer_chasing.go
echo ""
echo "=== Go Virtual Dispatch ==="
go run virtual_dispatch.go
echo ""
echo "=== Go Allocation ==="
go run allocation.go
```

---

## Expected Results (ballpark)

Absolute ns numbers vary by CPU, memory speed, OS scheduling, turbo behavior, and background load.
Focus on **ratios** and **directionality**.

### Pointer Chasing

* Pointer-chasing loop: typically several ns/element (dependent on cache miss rate)
* Contiguous values: typically sub-ns to ~1ns/element (prefetch + cache line reuse)

**Expectation:** Often **5–15×** slower for pointer chasing than contiguous values.

**Key point:** Both languages pay similar pointer-chasing costs.
The difference is how often your design pushes you into pointer graphs.

### Virtual / Interface Dispatch (isolated)

* Indirect dispatch: typically a few ns/call
* Direct calls: typically inlinable and much cheaper in tight loops

**Expectation:** Often **2–6×** overhead for indirect dispatch vs direct/inlinable calls,
depending on branch prediction and compiler success.

**Key point:** Vtable/itab overhead exists in both languages when you opt into dynamic dispatch.

### Allocation + Storage Pattern

* Heap allocation + pointer storage: materially more expensive than value storage in one contiguous block
* The ratio depends heavily on allocator behavior and object size

**Expectation:** Often **~1.5–6×** difference in "allocation + store" patterns.

**Key point:** This measures combined pressure: allocation + pointer topology + storage locality.

---

## Key Findings

The thesis these benchmarks support:

* **Pointer chasing dominates** when you build pointer graphs over large datasets.
* **Indirect dispatch costs** (vtable / interface) matter in hot loops and are similar in both languages.
* **Allocation costs compound** with storage topology: allocating many independent objects + storing pointers
  creates both allocator/GC pressure and cache-unfriendly traversal.

In classic C++ inheritance-heavy designs, these costs frequently appear together:

* dynamic dispatch + pointers + scattered objects

Go lets you choose when to pay them:

* concrete types and contiguous slices are the default pattern
* interfaces and pointer-heavy graphs are opt-in

**Important:** These costs can compound, but they do not multiply as a universal constant.
Real workloads overlap latency (cache miss + branch mispredict + pipelining) and shift bottlenecks.
The point is directionality and forcing functions, not a single "20×" number.

---

## Benchmarking Notes

For more consistent numbers:

* Close heavy background processes
* Run each benchmark multiple times
* Prefer a plugged-in laptop / stable power mode
* Avoid LTO for C++ unless you explicitly intend it
* Consider pinning to one CPU core (`taskset`) if you want lower variance

---

## System Requirements

* C++17 compiler: GCC 7+ or Clang 5+ (newer is better)
* Go 1.20+ recommended
* x86_64 or ARM64 CPU

---

## Citation

These benchmarks support the blog post:

[https://blog.blackwell-systems.com/posts/go-structs-not-cpp-classes/](https://blog.blackwell-systems.com/posts/go-structs-not-cpp-classes/)
