# Vertex-Cover-Algorithm-for-Optimal-Security-Camera-Placement

## Project Objective

The primary objective of this project is to significantly enhance security measures at local traffic intersections by strategically deploying security camera installations. 

## Vertex Cover Problem

In graph theory, a **vertex cover** is a subset \( U \) of vertices in a graph \( G = (V, E) \) such that for every edge \( (u, v) \) in \( E \), at least one of the vertices \( u \) or \( v \) (or both) is included in \( U \).

## Approach

This project explores various algorithms specifically designed to address the Vertex Cover problem. Our approach involves:

1. **Encoding Vertex Cover Instances:** We encode vertex cover instances into CNF-SAT clauses within the given graph. This process is facilitated by the use of MiniSat.
2. **Approximation Algorithms:** We apply two distinct approximation algorithms:
   - **Approx-VC-1**
   - **Approx-VC-2**
   Both algorithms are instrumental in accurately calculating the vertex cover.

## Runtime Assessment

To evaluate the runtime of our algorithms, we use the `pthread_getcpuclockid()` utility, which provides precise CPU execution time measurements for each algorithm thread.

## Additional Details

- The `CMakeLists.txt` file has been edited.
- Execute the file `ece650-prj.cpp` to run the project.
- A1, A2, A3, and A4 are integral components of this project, each contributing to the final outcome. All components are interrelated.

## How to Run

1. Ensure that the `CMakeLists.txt` file is properly configured.
2. Compile the project and run the `ece650-prj.cpp` file.

## Dependencies

- [MiniSat](http://minisat.se/)
- C++ Compiler
- CMake
