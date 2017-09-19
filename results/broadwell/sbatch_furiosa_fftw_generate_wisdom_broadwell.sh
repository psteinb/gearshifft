#!/bin/bash
#SBATCH -J gearshifftFFTWWisdom
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=110:00:00
#SBATCH --mem=128000M
#SBATCH --array 0-1
#SBATCH --partition=gpu
#SBATCH -o gearshifftFFTWWisdom-%A_%a.log

VVERS=3.3.6-pl1
VERS=fftw-${VVERS}

#module load gcc/5.3.0 fftw/${VVERS}-brdw

k=$SLURM_ARRAY_TASK_ID

if [ $k -eq 0 ]; then

    module load fftw/${VVERS}-bdw-sp
    fftwf-wisdom -v -c -n -T 40 -o ~/development/gearshifft/results/broadwell/fftwf_wisdom_${VVERS}_bdw__.txt

elif [ $k -eq 1 ]; then

    module load fftw/${VVERS}-bdw-dp
    fftw-wisdom -v -c -n -T 40 -o ~/development/gearshifft/results/broadwell/fftw_wisdom_${VVERS}_bdw__.txt
fi