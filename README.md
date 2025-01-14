# HiOp - HPC solver for optimization
![tests](https://github.com/LLNL/hiop/workflows/tests/badge.svg)

HiOp is an optimization solver for solving certain mathematical optimization problems expressed as nonlinear programming problems. HiOp is a lightweight HPC solver that leverages application's existing data parallelism to parallelize the optimization iterations by using specialized parallel linear algebra kernels.

Please cite the user manual whenever HiOp is used:
```
@TECHREPORT{hiop_techrep,
  title={{HiOp} -- {U}ser {G}uide},
  author={Petra, Cosmin G. and Chiang, NaiYuan and Jingyi Wang},
  year={2018},
  institution = {Center for Applied Scientific Computing, Lawrence Livermore National Laboratory},
  number = {LLNL-SM-743591}
}
```
In addition, when using the quasi-Newton solver please cite:
```
@ARTICLE{Petra_18_hiopdecomp,
title = {A memory-distributed quasi-Newton solver for nonlinear programming problems with a small number of general constraints},
journal = {Journal of Parallel and Distributed Computing},
volume = {133},
pages = {337-348},
year = {2019},
issn = {0743-7315},
doi = {https://doi.org/10.1016/j.jpdc.2018.10.009},
url = {https://www.sciencedirect.com/science/article/pii/S0743731518307731},
author = {Cosmin G. Petra},
}
```
and when using the the PriDec solver please cite:
```
@article{wang2022,
  archivePrefix = {arXiv},
  eprint = {arXiv:2204.09631},
  author = {J. Wang and C. G. Petra},
  title = {An optimization algorithm for nonsmooth nonconvex problems with upper-$C^2$ objective},
  publisher = {arXiv},
  year = {2022},
  journal={ (submitted) },
}
@INPROCEEDINGS{wang2021,
  author={Wang, Jingyi and Chiang, Nai-Yuan and Petra, Cosmin G.},
  booktitle={2021 20th International Symposium on Parallel and Distributed Computing (ISPDC)}, 
  title={An asynchronous distributed-memory optimization solver for two-stage stochastic programming problems}, 
  year={2021},
  volume={},
  number={},
  pages={33-40},
  doi={10.1109/ISPDC52870.2021.9521613}
}
```

## Build/install instructions
HiOp uses a CMake-based build system. A standard build can be done by invoking in the 'build' directory the following 
```shell 
$> cmake ..
$> make 
$> make test
$> make install
```
This sequence will build HiOp, run integrity and correctness tests, and install the headers and the library in the directory '_dist-default-build' in HiOp's root directory. 

Command `make test` runs extensive tests of the various modules of HiOp to check integrity and correctness. The tests suite range from unit testing to solving concrete optimization problems and checking the performance of HiOp solvers on these problems against known solutions. By default `make test` runs `mpirun` locally, which may not work on some HPC machines. For these HiOp allows using `bsub` to schedule `make test` on the compute nodes; to enable this, the use should use *-DHIOP_TEST_WITH_BSUB=ON* with cmake when building and run `make test` in a bsub shell session, for example,
```
bsub -P your_proj_name -nnodes 1 -W 30
make test
CTRL+D
```

The installation can be customized using the standard CMake options. For example, one can provide an alternative installation directory for HiOp by using 
```sh
$> cmake -DCMAKE_INSTALL_PREFIX=/usr/lib/hiop ..'
```


### Selected HiOp-specific build options
* Enable/disable MPI: *-DHIOP_USE_MPI=[ON/OFF]* (by default ON)
* GPU support: *-DHIOP_USE_GPU=ON*. MPI can be either off or on. For more build system options related to GPUs, see "Dependencies" section below.
* Enable/disable "developer mode" build that enforces more restrictive compiler rules and guidelines: *-DHIOP_DEVELOPER_MODE=ON*. This option is by default off.
* Additional checks and self-diagnostics inside HiOp meant to detect abnormalities and help to detect bugs and/or troubleshoot problematic instances: *-DHIOP_DEEPCHECKS=[ON/OFF]* (by default ON). Disabling HIOP_DEEPCHECKS usually provides 30-40% execution speedup in HiOp. For full strength, it is recommended to use HIOP_DEEPCHECKS with debug builds. With non-debug builds, in particular the ones that disable the assert macro, HIOP_DEEPCHECKS does not perform all checks and, thus, may overlook potential issues.

For example:
```shell 
$> cmake -DHIOP_USE_MPI=ON -DHIOP_DEEPCHECKS=ON ..
$> make 
$> make test
$> make install
```


### Other useful options to use with CMake
* *-DCMAKE_BUILD_TYPE=Release* will build the code with the optimization flags on
* *-DCMAKE_CXX_FLAGS="-O3"* will enable a high level of compiler code optimization

### Dependencies

A complete list of dependencies is maintained [here](https://github.com/spack/spack/blob/develop/var/spack/repos/builtin/packages/hiop/package.py).

For a minimal build, HiOp requires LAPACK and BLAS. These dependencies are automatically detected by the build system. MPI is optional and by default enabled. To disable use cmake option '-DHIOP_USE_MPI=OFF'.

HiOp has support for NVIDIA **GPU-based computations** via CUDA and Magma. To enable the use of GPUs, use cmake with '-DHIOP_USE_GPU=ON'. The build system will automatically search for CUDA Toolkit. For non-standard CUDA Toolkit installations, use '-DHIOP_CUDA_LIB_DIR=/path' and '-DHIOP_CUDA_INCLUDE_DIR=/path'. For "very" non-standard CUDA Toolkit installations, one can specify the directory of cuBlas libraries as well with '-DHIOP_CUBLAS_LIB_DIR=/path'.

### Using RAJA and Umpire portability libraries

Portability libraries allow running HiOp's linear algebra either on host (CPU) or a device (GPU). RAJA and Umpire are disabled by default. You can turn them on together by passing `-DHIOP_USE_RAJA=ON` to CMake. If the two libraries are not automatically found, specify their installation directories like this:
```shell
$> cmake -DHIOP_USE_RAJA=ON -DRAJA_DIR=/path/to/raja/dir -Dumpire_DIR=/path/to/umpire/dir
```
If the GPU support is enabled, RAJA will run all HiOp linear algebra kernels on GPU, otherwise RAJA will run the kernels on CPU using an OpenMP execution policy.

### Support for GPU computations

When GPU support is on, HiOp requires Magma linear solver library and CUDA Toolkit. Both are detected automatically in most cases. The typical cmake command to enable GPU support in HiOp is
```shell 
$> cmake -DHIOP_USE_GPU=ON ..
```

When Magma is not detected, one can specify its location by passing `-DHIOP_MAGMA_DIR=/path/to/magma/dir` to cmake.

For custom CUDA Toolkit installations, the locations to the (missing/not found) CUDA libraries can be specified to cmake via `-DNAME=/path/cuda/directory/lib`, where `NAME` can be any of  
```
CUDA_cublas_LIBRARY
CUDA_CUDART_LIBRARY
CUDA_cudadevrt_LIBRARY
CUDA_cusparse_LIBRARY
CUDA_cublasLt_LIBRARY
CUDA_nvblas_LIBRARY
CUDA_culibos_LIBRARY
 ```
Below is an example for specifiying `cuBlas`, `cuBlasLt`, and `nvblas` libraries, which were `NOT_FOUND` because of a non-standard CUDA Toolkit instalation:
```shell 
$> cmake -DHIOP_USE_GPU=ON -DCUDA_cublas_LIBRARY=/usr/local/cuda-10.2/targets/x86_64-linux/lib/lib64 -DCUDA_cublasLt_LIBRARY=/export/home/petra1/work/installs/cuda10.2.89/targets/x86_64-linux/lib/ -DCUDA_nvblas_LIBRARY=/export/home/petra1/work/installs/cuda10.2.89/targets/x86_64-linux/lib/ .. && make -j && make install
```

A detailed example on how to compile HiOp straight of the box on `summit.olcf.ornl.gov` is available [here](README_summit.md).

RAJA and UMPIRE dependencies are usually detected by HiOp's cmake build system. 

### Kron reduction

Kron reduction functionality of HiOp is disabled by default. One can enable it by using 
```shell
$> rm -rf *; cmake -DHIOP_WITH_KRON_REDUCTION=ON -DUMFPACK_DIR=/Users/petra1/work/installs/SuiteSparse-5.7.1 -DMETIS_DIR=/Users/petra1/work/installs/metis-4.0.3 .. && make -j && make install
```
Metis is usually detected automatically and needs not be specified under normal circumstances.

UMFPACK (part of SuiteSparse) and METIS need to be provided as shown above.

# Interfacing with HiOp 

HiOp supports three types of optimization problems, each with a separate input formats in the form of the C++ interfaces `hiopInterfaceDenseConstraints`,`hiopInterfaceSparse` and `hiopInterfaceMDS`. These interfaces are specified in [hiopInterface.hpp](src/Interface/hiopInterface.hpp) and documented and discussed as well in the [user manual](doc/hiop_usermanual.pdf).

*`hiopInterfaceDenseConstraints` interface* supports NLPs with **billions** of variables with and without bounds but only limited number (<100) of general, equality and inequality constraints. The underlying algorithm is a limited-memory quasi-Newton interior-point method and generally scales well computationally (but it may not algorithmically) on thousands of cores. This interface uses MPI for parallelization

*`hiopInterfaceSparse` interface* supports general sparse and large-scale NLPs. This functionality is similar to that of the state-of-the-art [Ipopt](https://github.com/coin-or/Ipopt) (without being as robust and flexible as Ipopt is). Acceleration for this class of problems can be achieved via OpenMP or CUDA, however, this is work in progress and you are encouraged to contact HiOp's developers for up-to-date information.

*`hiopInterfaceMDS` interface* supports mixed dense-sparse NLPs and achives parallelization using GPUs and RAJA portability abstraction layer. 

More information on the HiOp interfaces are [here](src/Interface/README.md).

## Running HiOp tests and applications

HiOp is using NVBlas library when built with CUDA support. If you don't specify
location of the `nvblas.conf` configuration file, you may get an annoying
warnings. HiOp provides default `nvblas.conf` file and installs it at the same
location as HiOp libraries. To use it, set environment variable as
```bash
$ export NVBLAS_CONFIG_FILE=<hiop install dir>/lib/nvblas.conf
```
or, if you are using C-shell, as
```shell
$ setenv NVBLAS_CONFIG_FILE <hiop install dir>/lib/nvblas.conf
```

## Existing issues
Users are highly encouraged to report any issues they found from using HiOp.
One known issue is that there is some minor inconsistence between HiOp and linear package STRUMPACK.
When STRUMPACK is compiled with MPI (and Scalapack), user must set flag `HIOP_USE_MPI` to `ON` when compiling HiOp.
Otherwise HiOp won't load MPI module and will return an error when links to STRUMPACK, since the later one requires a valid MPI module. 
Similarly, if both Magma and STRUMPACK are linked to HiOp, user must guarantee the all the packages are compiled by the same CUDA compiler.
User can check other issues and their existing status from https://github.com/LLNL/hiop 


## Acknowledgments

HiOp has been developed under the financial support of: 
- Department of Energy, Office of Advanced Scientific Computing Research (ASCR): Exascale Computing Program (ECP) and Applied Math Program.
- Department of Energy, Advanced Research Projects Agency-Energy (ARPA‑E)
- Lawrence Livermore National Laboratory Institutional Scientific Capability Portfolio (ISCP)
- Lawrence Livermore National Laboratory, through the LDRD program

# Contributors

HiOp is written by Cosmin G. Petra (petra1@llnl.gov), Nai-Yuan Chiang (chiang7@llnl.gov), and Jingyi "Frank" Wang (wang125@llnl.gov) from LLNL and has received important contributions from Asher Mancinelli (PNNL), Slaven Peles (ORNL), Cameron Rutherford (PNNL), Jake K. Ryan (PNNL), and Michel Schanen (ANL).

# Copyright

Copyright (c) 2017-2021, Lawrence Livermore National Security, LLC. All rights reserved. Produced at the Lawrence Livermore National Laboratory. LLNL-CODE-742473. HiOp is free software; you can modify it and/or redistribute it under the terms of the BSD 3-clause license. See [COPYRIGHT](/COPYRIGHT) and [LICENSE](/LICENSE) for complete copyright and license information.
 

