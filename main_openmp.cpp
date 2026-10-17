#include <iostream>
#include <cmath>
#include <mpi.h>
#include <fstream>

// Alias for unsigned 64-bit integer as recommended
typedef unsigned long ULONG;

// LCG Constants
const ULONG a = 1664525;
const ULONG c = 1013904223;
const ULONG m = 4294967296; // 2^32

// Gamma integrand function: f(t) = (log(1/t))^(x-1)
double gamma_integrand(double t, double x) {
    if (t <= 0.0 || t >= 1.0) return 0.0; // Avoid domain errors
    return pow(log(1.0 / t), x - 1.0);
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Assignment parameters
    const ULONG N_TOTAL = 3000000000;
    ULONG N_local = N_TOTAL / size;
    double target_x = 5.0; // Testing Gamma(5) = 24

    // 1. Calculate Leapfrog Constants A and C
    ULONG A = 1;
    ULONG C = 0;
    ULONG current_a_power = 1;
    for (int i = 0; i < size; ++i) {
        A = (A * a) % m;
        C = (C + c * current_a_power) % m;
        current_a_power = (current_a_power * a) % m;
    }

    // 2. Master generates initial seeds and distributes them
    ULONG my_seed = 0;
    double comm_start = 0.0, comm_end = 0.0;

    if (rank == 0) {
        ULONG* initial_seeds = new ULONG[size];
        ULONG current_seed = 12345;
        for (int i = 0; i < size; ++i) {
            initial_seeds[i] = current_seed;
            current_seed = (a * current_seed + c) % m;
        }

        comm_start = MPI_Wtime();
        MPI_Scatter(initial_seeds, 1, MPI_UNSIGNED_LONG, &my_seed, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
        comm_end = MPI_Wtime();

        std::cout << " Interprocess Communication Time for Scatter: " << (comm_end - comm_start) << " seconds.\n";
        delete[] initial_seeds;
    } else {
        MPI_Scatter(NULL, 1, MPI_UNSIGNED_LONG, &my_seed, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
    }

    // 3. Parallel Monte Carlo Integration
    double start_time = MPI_Wtime();
    double local_sum = 0.0;
    ULONG current_val = my_seed;

    for (ULONG i = 0; i < N_local; ++i) {
        // Convert to floating point between 0 and 1
        double t = (double)current_val / (double)m;
        local_sum += gamma_integrand(t, target_x);

        // Leapfrog to the next random number
        current_val = (A * current_val + C) % m;
    }

    // 4. Reduce results to Master
    double global_sum = 0.0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    // 5. Output Results
    if (rank == 0) {
        double total_computation_time = end_time - start_time;
        double baseline_time = 0.0;

            // Check if this is the 1-processor run
            if (size == 1) {
                // Save the baseline time to a file for future runs to read
                std::ofstream outfile("baseline.txt");
                if (outfile.is_open()) {
                    outfile << total_computation_time;
                    outfile.close();
                }
                baseline_time = total_computation_time;
            } else {
                // This is a parallel run. Read the saved baseline time from the file.
                std::ifstream infile("baseline.txt");
                if (infile.is_open()) {
                    infile >> baseline_time;
                    infile.close();
                } else {
                    std::cerr << "Warning: Could not open baseline.txt\n";
                    baseline_time = total_computation_time; // Fallback
                }
            }

            // Calculate dynamic metrics
            double speedup = baseline_time / total_computation_time;
            double efficiency = (speedup / size) * 100.0;
        double integral_estimate = global_sum / (double)N_TOTAL;
        std::cout << " Number of Processor                        : " << size << "\n";
        std::cout << " Samples (N)                                : " << N_TOTAL << "\n";
        std::cout << " Estimated Gamma(" << target_x << ")                         : " << integral_estimate << "\n";
        std::cout << " Total Computation Time                     : " << (end_time - start_time) << " seconds.\n";
        std::cout << " Speedup (S)                                : " << speedup << "x\n";
        std::cout << " Efficiency (E)                             : " << efficiency << "%\n";
        std::cout << "==============================================================================\n";
    }

    MPI_Finalize();
    return 0;
}