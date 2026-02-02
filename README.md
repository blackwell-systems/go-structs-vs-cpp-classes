# Go Structs vs C++ Classes: Performance Benchmarks

Benchmarks to substantiate claims in the blog post "Go Structs Are Not C++ Classes: 7 Hardware-Level Differences"

## Benchmarks

### 1. Pointer Chasing vs Contiguous Memory

Tests the cache locality difference between:
- C++: Array of pointers to heap objects (scattered memory)
- Go: Array of value types (contiguous memory)

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

Tests the method call overhead difference between:
- C++: Virtual methods (vtable lookup)
- C++: Concrete types (static dispatch, can be inlined)
- Go: Interface types (dynamic dispatch)
- Go: Concrete types (static dispatch, can be inlined)

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

Tests the allocation overhead difference between:
- C++: Heap allocation (new + pointer storage in vector)
- C++: Stack-based storage (value storage in vector)
- Go: Heap allocation (pointer slice)
- Go: Value storage (contiguous slice)

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
   - C++ pointer array: ~100ns per element (cache miss)
   - C++ value array: ~1-5ns per element (cache hit)
   - Go value array: ~1-5ns per element (cache hit)
   - **Expected:** 20-100× difference

2. **Virtual Dispatch:**
   - C++ virtual: ~10ns per call (vtable + indirect branch)
   - C++ static: <1ns per call (inlined)
   - Go interface: ~10ns per call (similar to C++ virtual)
   - Go concrete: <1ns per call (inlined)
   - **Expected:** 10-20× difference

3. **Allocation:**
   - C++ heap (new + pointer storage): ~35ns per allocation
   - C++ stack-based (vector values): ~7ns per allocation
   - Go heap (pointer slice): ~100ns per allocation
   - Go value slice: ~60ns per allocation
   - **Expected:** 2-5× difference (value storage vs pointer storage)

## Notes

- Compile C++ with `-O3` for maximum optimization
- Results vary by CPU, memory speed, and system load
- Run multiple times for consistent measurements
- Benchmarks isolated to measure single factors
- Real-world performance depends on workload mix

## System Requirements

- C++17 compatible compiler (g++ 7+, clang 5+)
- Go 1.16+ (for module support)
- x86_64 or ARM64 CPU

## Citation

These benchmarks support the blog post at:
https://blog.blackwell-systems.com/posts/go-structs-not-cpp-classes/
