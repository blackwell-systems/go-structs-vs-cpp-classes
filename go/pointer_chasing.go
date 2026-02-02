// Benchmark 1: Go value semantics (contiguous memory)
// Run: go run pointer_chasing.go
//
// This shows Go CAN use pointers (scattered memory) but DOESN'T REQUIRE them.
// Unlike C++ inheritance which FORCES pointer arrays, Go lets you choose:
//   - []Point for values (fast, contiguous)
//   - []*Point for pointers (slower, but sometimes needed)
//   - []Shape for polymorphism (opt-in cost)

package main

import (
	"fmt"
	"time"
)

type Point struct {
	X, Y int
}

// Measure value array (contiguous memory)
func benchmarkValueArray(n int, iterations int) time.Duration {
	points := make([]Point, n)
	
	// Initialize
	for i := 0; i < n; i++ {
		points[i] = Point{X: i, Y: i}
	}
	
	start := time.Now()
	
	for iter := 0; iter < iterations; iter++ {
		sum := 0
		for i := 0; i < n; i++ {
			sum += points[i].X + points[i].Y
		}
		// Prevent optimization
		if sum < 0 {
			fmt.Println(sum)
		}
	}
	
	return time.Since(start)
}

// Measure pointer array (to simulate scattered memory)
func benchmarkPointerArray(n int, iterations int) time.Duration {
	points := make([]*Point, n)
	
	// Allocate on heap (scattered)
	for i := 0; i < n; i++ {
		points[i] = &Point{X: i, Y: i}
	}
	
	start := time.Now()
	
	for iter := 0; iter < iterations; iter++ {
		sum := 0
		for i := 0; i < n; i++ {
			sum += points[i].X + points[i].Y
		}
		// Prevent optimization
		if sum < 0 {
			fmt.Println(sum)
		}
	}
	
	return time.Since(start)
}

func main() {
	const n = 1000000  // 1 million points
	const iterations = 100
	
	fmt.Println("Benchmarking Go value semantics vs pointer arrays")
	fmt.Printf("Elements: %d\n", n)
	fmt.Printf("Iterations: %d\n\n", iterations)
	
	fmt.Println("KEY DIFFERENCE FROM C++:")
	fmt.Println("Go lets you choose based on your needs:")
	fmt.Println("  []Point   - Values (contiguous, cache-friendly)")
	fmt.Println("  []*Point  - Pointers (scattered, but sometimes needed)")
	fmt.Println("  []Shape   - Interfaces (opt-in polymorphism)")
	fmt.Println("\nC++ inheritance FORCES pointer arrays:")
	fmt.Println("  vector<Shape*> shapes;  // Required for polymorphism")
	fmt.Println("  vector<Shape> shapes;   // Fails (object slicing)")
	fmt.Println()
	
	// Warm up
	benchmarkValueArray(1000, 10)
	benchmarkPointerArray(1000, 10)
	
	// Benchmark value array
	valueTime := benchmarkValueArray(n, iterations)
	valueMicros := valueTime.Microseconds()
	valuePerElement := valueTime.Nanoseconds() / int64(n*iterations)
	
	fmt.Println("Value array ([]Point - Go default for concrete types):")
	fmt.Printf("  Total time: %.2f ms\n", float64(valueMicros)/1000.0)
	fmt.Printf("  Time per element: %d ns\n\n", valuePerElement)
	
	// Benchmark pointer array
	pointerTime := benchmarkPointerArray(n, iterations)
	pointerMicros := pointerTime.Microseconds()
	pointerPerElement := pointerTime.Nanoseconds() / int64(n*iterations)
	
	fmt.Println("Pointer array ([]*Point - possible but not required):")
	fmt.Printf("  Total time: %.2f ms\n", float64(pointerMicros)/1000.0)
	fmt.Printf("  Time per element: %d ns\n\n", pointerPerElement)
	
	// Calculate difference
	speedup := float64(pointerTime) / float64(valueTime)
	fmt.Printf("Speedup: %.2fx faster for value semantics\n", speedup)
	fmt.Println("\nConclusion: Go makes the fast path (values) the default.")
	fmt.Println("C++ inheritance makes the slow path (pointers) mandatory.")
}
