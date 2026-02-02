// Benchmark 2: Virtual dispatch vs static dispatch (isolated)
// Compile: g++ -O3 -std=c++17 -o virtual_dispatch virtual_dispatch.cpp
// Run: ./virtual_dispatch
//
// CRITICAL: Both use contiguous arrays (not pointers) to isolate vtable overhead
// This measures ONLY virtual dispatch cost, not pointer chasing + vtable.

#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

// Virtual dispatch (contiguous storage with vtable)
struct VirtualCircle {
    int radius;
    
    virtual double area() const {
        return 3.14159 * radius * radius;
    }
    
    virtual ~VirtualCircle() = default;
};

// Static dispatch (contiguous storage, no vtable)
struct ConcreteCircle {
    int radius;
    
    double area() const {
        return 3.14159 * radius * radius;
    }
};

// Benchmark virtual dispatch (contiguous array, virtual methods)
long long benchmark_virtual(size_t n, int iterations) {
    std::vector<VirtualCircle> circles;
    circles.reserve(n);
    
    for (size_t i = 0; i < n; i++) {
        VirtualCircle c;
        c.radius = i;
        circles.push_back(c);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < iterations; iter++) {
        double sum = 0;
        for (size_t i = 0; i < n; i++) {
            sum += circles[i].area();  // Virtual call (vtable lookup)
        }
        // Prevent optimization
        if (sum < 0) std::cout << sum;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

// Benchmark static dispatch (contiguous array, no virtual)
long long benchmark_static(size_t n, int iterations) {
    std::vector<ConcreteCircle> circles;
    circles.reserve(n);
    
    for (size_t i = 0; i < n; i++) {
        circles.push_back({static_cast<int>(i)});
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < iterations; iter++) {
        double sum = 0;
        for (size_t i = 0; i < n; i++) {
            sum += circles[i].area();  // Static call (can be inlined)
        }
        // Prevent optimization
        if (sum < 0) std::cout << sum;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

int main() {
    const size_t n = 10000000;  // 10 million calls
    const int iterations = 10;
    
    std::cout << "Benchmarking virtual dispatch vs static dispatch (isolated)\n";
    std::cout << "Elements: " << n << "\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Total calls: " << n * iterations << "\n";
    std::cout << "\nIMPORTANT: Both use contiguous arrays (vector<T>, not vector<T*>)\n";
    std::cout << "This isolates vtable overhead without pointer chasing.\n\n";
    
    // Warm up
    benchmark_virtual(1000, 10);
    benchmark_static(1000, 10);
    
    // Benchmark virtual dispatch
    auto virtual_time = benchmark_virtual(n, iterations);
    auto virtual_ms = virtual_time / 1000000.0;
    auto virtual_per_call = virtual_time / (n * iterations);
    
    std::cout << "Virtual dispatch (contiguous array + vtable):\n";
    std::cout << "  Total time: " << virtual_ms << " ms\n";
    std::cout << "  Time per call: " << virtual_per_call << " ns\n\n";
    
    // Benchmark static dispatch
    auto static_time = benchmark_static(n, iterations);
    auto static_ms = static_time / 1000000.0;
    auto static_per_call = static_time / (n * iterations);
    
    std::cout << "Static dispatch (contiguous array, no vtable):\n";
    std::cout << "  Total time: " << static_ms << " ms\n";
    std::cout << "  Time per call: " << static_per_call << " ns\n\n";
    
    // Calculate speedup
    double speedup = static_cast<double>(virtual_time) / static_time;
    std::cout << "Speedup: " << speedup << "x faster for static dispatch\n";
    std::cout << "\nNote: This measures pure vtable overhead.\n";
    std::cout << "In real inheritance hierarchies, you also pay pointer chasing cost\n";
    std::cout << "(see pointer_chasing benchmark).\n";
    
    return 0;
}
