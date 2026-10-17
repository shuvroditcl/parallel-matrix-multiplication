#!/bin/bash
#SBATCH --job-name=ass1_amdahl
#SBATCH --ntasks=40
#SBATCH --time=00:10:00

# 1. Load the MPI environment
module load openmpi

# 2. Compile the C++ code automatically
mpic++ -O3 -o main_openmp main_openmp.cpp

# 3. Run the tests for Amdahl's Law (Constant Problem Size)
echo "========== 1 PROCESSOR =========="
mpirun --mca btl ^openib -np 1 ./main_openmp

echo "========== 4 PROCESSORS ========="
mpirun --mca btl ^openib -np 4 ./main_openmp

echo "========== 8 PROCESSORS ========="
mpirun --mca btl ^openib -np 8 ./main_openmp

echo "========== 12 PROCESSORS ========"
mpirun --mca btl ^openib -np 12 ./main_openmp

echo "========== 16 PROCESSORS ========"
mpirun --mca btl ^openib -np 16 ./main_openmp

echo "========== 20 PROCESSORS ========"
mpirun --mca btl ^openib -np 20 ./main_openmp

echo "========== 24 PROCESSORS ========"
mpirun --mca btl ^openib -np 24 ./main_openmp

echo "========== 28 PROCESSORS ========"
mpirun --mca btl ^openib -np 28 ./main_openmp

echo "========== 32 PROCESSORS ========"
mpirun --mca btl ^openib -np 32 ./main_openmp

echo "========== 36 PROCESSORS ========"
mpirun --mca btl ^openib -np 36 ./main_openmp

echo "========== 40 PROCESSORS ========"
mpirun --mca btl ^openib -np 40 ./main_openmp