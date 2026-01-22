# Contributing to Ternary GCC Plugin

## How to Contribute

1. Fork the repository.
2. Create a feature branch.
3. Make your changes and add any new tests or helpers.
4. Run `make test` (and `make check-helper-docs` if you touched the helper ABI).
5. Submit a PR with a concise description of the behavioral changes and any test results.

## Guidelines

- Follow GCC plugin best practices.
- Run `tools/check_helper_docs.py` (via `make check-helper-docs`) whenever you add or update helpers so the documentation stays in sync with `include/ternary_runtime.h`.
- Add regression tests whenever the helper ABI or runtime behavior changes.
- Reference the updated README sections when describing functionality and compatibility.

## Building with CMake

```bash
mkdir build && cd build
cmake ..
make
```
