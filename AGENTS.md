# Repository Guidelines

## Project Structure & Module Organization

`include/` contains the header-only vISA template library; `gen_visa_templates.hpp` is its umbrella include, while `lsc.hpp`, `regmap.hpp`, `systolic.hpp`, and `gateway.hpp` provide the main APIs. Shared SYCL queue helpers live in `sycl_misc.hpp` and `sycl_misc.cpp`. Standalone functional programs are under `test/`, and performance experiments are under `benchmark/`; both directories have their own Makefiles. `generate_lsc_untyped.py` regenerates the `include/list_*.list` descriptor tables. Treat `cxxopts.hpp` and `cutlass/` as vendored code, and avoid committing incidental compiler dumps or disassembly artifacts.

## Build, Test, and Development Commands

The build requires a `clang++` toolchain with Intel SYCL extensions and `clang-offload-bundler`.

```sh
make copy_verify              # build a root-level utility
make -C test all              # build all listed test executables
make -C test unit_test        # build one test
./test/unit_test              # run it on a compatible GPU
make -C benchmark all         # build all benchmarks
make -C benchmark gemm        # build one benchmark
make -C test clean            # remove generated test build files
```

Root and benchmark builds default to PVC; tests default to `bmg-g21-a0`. Override when needed, for example `make -C test unit_test ENABLE_AOT=pvc`. `BACKEND=CUDA` selects the configured `sm_86` path.

## Coding Style & Naming Conventions

Use C++17, two-space indentation, and opening braces on the same line; match nearby code where legacy formatting differs. Use `PascalCase` for types and template abstractions, `lowerCamelCase` for APIs such as `lscLoad`, and uppercase names for constants such as `SG_SZ`. Guard vISA-only code with `#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)`. No repository-wide formatter or linter is configured, so keep formatting-only changes out of functional patches. After descriptor changes, run `python3 generate_lsc_untyped.py` and review every generated table diff.

## Testing Guidelines

Tests are standalone SYCL executables rather than a test-framework suite; there is no coverage target. Name new cases `test_<feature>.cpp` or follow the nearest descriptive pattern, then add the executable and link rule to `test/Makefile`. Run the affected binary on the intended GPU and validate results explicitly, including edge shapes or subgroup sizes relevant to the change.

## Commit & Pull Request Guidelines

History favors short imperative subjects such as `Fix gemm example` and `Add dump state registers`. Most commits carry a `Signed-off-by` trailer; use `git commit -s`. Pull requests should explain the behavior and rationale, identify the compiler and AOT target, list exact build/run commands with results, and link any issue or reproducer. Include before/after measurements for performance changes and relevant generated-code or disassembly evidence for vISA changes.
