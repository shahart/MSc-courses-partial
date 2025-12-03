# Copilot / AI agent instructions for 13-backTrack

This repo contains multiple C programs and experimental drivers for binary constraint
satisfaction search (backtracking) experiments. The guidance below helps an AI coding
assistant become productive quickly by highlighting the project's architecture,
build/run workflow, important files, and repository-specific conventions.

**Architecture Summary**
- **One main experiment driver:** `mmn13.c` — the primary, parameterized solver that
  includes many `.inc` fragments (problem generators, checks, labeling functions).
- **Standalone example:** `main.c` contains a smaller, self-contained set of algorithms
  and is useful to understand the algorithmic structure (simple N-queens example).
- **Modular fragments:** `.inc` files (e.g. `bcssp.inc`, `checks.inc`, `bt_labels.inc`,
  `fc_labels.inc`, `ordering.inc`, `saveload.inc`, `problems.inc`, `macros.inc`) hold
  core building blocks and must be updated to change behavior.

**Build & Run (developer workflows)**
- Primary build is driven by `makefile`. It compiles with compile-time constants passed
  as `-D` flags. Example (same as makefile):

```bash
gcc -c mmn13.c -DVARS_NUM=20 -DDOMAIN_SIZE=7
gcc -o mmn13 mmn13.o
```

- NOTE: `makefile` currently runs `gcc -o mmn13. mmn13.o` (binary name ends with a dot).
  However `runmeParm.sh` expects `./mmn13`. If `mmn13` is not created, build the binary
  with `-o mmn13` (remove the trailing dot) so runtime scripts work as-is.

- Use the provided runner script `runmeParm.sh` (bash) to sweep parameters. Example:

```bash
./runmeParm.sh -d -alg fc -i 10 -p 0.15 -t 0.30 -timeout 5000
```

- You can also run `mmn13` directly. Minimal example:

```bash
./mmn13 -p 0.15 -t 0.3 -i 1 -alg fc
```

**Important runtime options** (see `mmn13.c` / `bad_options`):
- `-p <num>` density (probability two variables are constrained)
- `-t <num>` tightness (probability that an assignment is forbidden)
- `-i <num>` number of iterations (generate and solve multiple problems)
- `-s <file>` save generated problems
- `-l <file>` load problem from file instead of generating
- `-d` use dynamic variable ordering (affects FC* only)
- `-alg <name>` choose algorithm: `bt`, `bj`, `cbj`, `fc`, `fc_bj`, `fc_cbj`

**Project-specific conventions & patterns**
- The code relies on compile-time constants `VARS_NUM` and `DOMAIN_SIZE`. These are
  not defined in source files but passed via `-D` flags at compile time. Search for
  `-DVARS_NUM` in `makefile` for examples.
- Problem logic is split into `.inc` fragments that are included in `mmn13.c`. Edit
  these `.inc` files to change labeling strategies, ordering, or constraint checks.
- `QUEENS` is a compile-time specialization. Build with `-DQUEENS` to compile the
  queens generator path (see `makefile` and `mmn13.c` `#ifdef QUEENS`).
- Many historical C idioms are used: `mem.h`, `ftime`, and manual stack implementations.
  Expect non-modern APIs and platform-specific behaviors.

**Key files to inspect when making changes**
- `mmn13.c` — main, command-line parsing, orchestration, includes the algorithm hooks
- `makefile` — simple build; check output name (see note above)
- `runmeParm.sh` — parameter sweep driver and example of how the binary is invoked
- `*.inc` (e.g. `bcssp.inc`, `checks.inc`, `bt_labels.inc`, `fc_labels.inc`,
  `ordering.inc`, `saveload.inc`, `problems.inc`, `macros.inc`) — main algorithm
  building blocks: labeling, unlabeling, domain checks, generators, save/load
- `main.c` — smaller, easier-to-follow example implementation of similar algorithms

**Debugging & editing hints for an AI agent**
- When changing search logic, update or run small iterations (`-i 1`) and use `-s`
  to save generated problems for reproducibility.
- To debug, compile with `-g` and `-O0` and run with `gdb` or add `printf` traces.
  Example compile flags: `gcc -g -O0 -c mmn13.c -DVARS_NUM=20 -DDOMAIN_SIZE=7`
- Avoid large VARS_NUM and DOMAIN_SIZE while iterating — build-time constants affect
  static arrays and memory layout.

**What to check before pushing changes**
- If you change function signatures used across `.inc` fragments, ensure every
  `#include` site (`mmn13.c` or other drivers) remains consistent.
- Running `runmeParm.sh` requires a POSIX shell; on Windows use Git Bash or WSL.
- Verify the binary name after `make` — the runtime scripts expect `./mmn13`.

