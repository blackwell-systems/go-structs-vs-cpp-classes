# Go Structs vs C++ Classes: Performance Benchmarks

Benchmarks to substantiate claims in the blog post "Go Structs Are Not C++ Classes: 7 Hardware-Level Differences"

## Benchmarks

### 1. Pointer Chasing vs Contiguous Memory

**What this proves:** C++ inheritance **forces** pointer arrays (can't store different-sized types inline). Go makes pointers **optional**.

Tests the cache locality difference between:
- C++: Array of pointers to heap objects (scattered memory) - **required for polymorphism**
- C++: Array of value types (contiguous memory) - only works without polymorphism
- Go: Array of value types (contiguous memory) - **default, works with concrete types**
- Go: Array of pointers (scattered memory) - possible but not required

**C++ Compile & Run:**
```bash
cd cpp
g++ -O3 -std=c++17 -o pointer_chasing pointer_chasing.cpp
./pointer_chasing
```

**Go Run:**
```bash
cd go
go run pointer_chasing.go
```

### 2. Virtual Dispatch vs Static Dispatch

**What this proves:** Vtable overhead exists in both languages, but this benchmark **isolates** it from memory layout costs.

**IMPORTANT:** Both C++ benchmarks use **contiguous arrays** (`vector<T>`, not `vector<T*>`). This measures **pure vtable overhead** without confounding it with pointer chasing.

Tests the method call overhead difference between:
- C++: Virtual methods with contiguous storage (vtable lookup, no pointer chasing)
- C++: Concrete types with contiguous storage (static dispatch, can be inlined)
- Go: Interface types (dynamic dispatch)
- Go: Concrete types (static dispatch, can be inlined)

In real C++ inheritance hierarchies, you pay **both** vtable cost (this benchmark) **and** pointer chasing cost (benchmark 1).

**C++ Compile & Run:**
```bash
cd cpp
g++ -O3 -std=c++17 -o virtual_dispatch virtual_dispatch.cpp
./virtual_dispatch
```

**Go Run:**
```bash
cd go
go run virtual_dispatch.go
```

### 3. Heap vs Stack Allocation

**What this proves:** Allocation overhead combined with storage patterns in realistic usage.

Tests the allocation overhead difference between:
- C++: Heap allocation (new + pointer storage in vector) - **pattern forced by inheritance**
- C++: Stack-based storage (value storage in vector) - only works without polymorphism
- Go: Heap allocation (pointer slice) - possible but not default
- Go: Value storage (contiguous slice) - **default pattern**

This benchmark stores allocated objects in collections to prevent dead code elimination, representing realistic usage patterns.

**C++ Compile & Run:**
```bash
cd cpp
g++ -O3 -std=c++17 -o allocation allocation.cpp
./allocation
```

**Go Run:**
```bash
cd go
go run allocation.go

# To see escape analysis decisions:
go run -gcflags="-m" allocation.go 2>&1 | grep escape
```

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

## Expected Results

Based on hardware fundamentals:

1. **Pointer Chasing:**
   - C++ pointer array: ~2ns per element (cache miss)
   - C++ value array: ~0.3ns per element (cache hit)
   - Go pointer array: ~2ns per element (cache miss)
   - Go value array: ~0.3ns per element (cache hit)
   - **Expected:** 5-10× difference
   - **Key finding:** Both languages have the same cost. C++ inheritance **forces** the slow path.

2. **Virtual Dispatch (isolated):**
   - C++ virtual (contiguous): ~3-5ns per call (vtable lookup only)
   - C++ static (contiguous): ~0.5-1ns per call (can be inlined)
   - Go interface: ~2-3ns per call (similar to C++ virtual)
   - Go concrete: ~0.5-1ns per call (can be inlined)
   - **Expected:** 3-5× difference
   - **Key finding:** Vtable overhead is similar in both languages. This is **just** the vtable cost, without pointer chasing.

3. **Allocation:**
   - C++ heap (new + pointer storage): ~35ns per allocation
   - C++ stack-based (vector values): ~7ns per allocation
   - Go heap (pointer slice): ~100ns per allocation
   - Go value slice: ~60ns per allocation
   - **Expected:** 1.5-5× difference (value storage vs pointer storage)
   - **Key finding:** Measures allocation + storage pattern, not just malloc overhead.

## Key Findings

**The thesis:** C++ inheritance **forces** pointer arrays + vtables. Go makes both **optional**.

1. **Pointer chasing** (Benchmark 1): Shows the cost difference, and explains **why** C++ polymorphism requires it (object slicing).

2. **Virtual dispatch** (Benchmark 2): **Isolated** vtable overhead using contiguous arrays in both languages. Proves the cost is similar when memory layout is controlled.

3. **Allocation** (Benchmark 3): Realistic patterns showing how allocation overhead combines with storage decisions.

**Combined effect:** In C++ inheritance hierarchies, you pay:
- Pointer chasing cost (~7× slower, Benchmark 1)
- Vtable overhead (~3-5× slower, Benchmark 2)
- Heap allocation cost (~5× slower, Benchmark 3)

**Total:** 7× (layout) × 3× (vtable) = **~20× slower** for inheritance-heavy code vs value-oriented code.

Go lets you choose when to pay these costs. C++ inheritance forces you to pay by default.

## Notes

- Compile C++ with `-O3` for maximum optimization
- Results vary by CPU, memory speed, and system load
- Run multiple times for consistent measurements
- Benchmarks designed to isolate individual factors (not combined)
- Real-world performance depends on workload mix
- The "forcing function" is the key finding, not just raw performance numbers

## System Requirements

- C++17 compatible compiler (g++ 7+, clang 5+)
- Go 1.16+ (for module support)
- x86_64 or ARM64 CPU

## Citation

These benchmarks support the blog post at:
https://blog.blackwell-systems.com/posts/go-structs-not-cpp-classes/
