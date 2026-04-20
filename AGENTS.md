# Repository Guidelines

## Project Structure & Module Organization

This repository is a coursework archive, organized by course and assignment rather than as a single buildable application. Top-level folders such as `20364-compilation-2001`, `20554-java-2004`, `22918-dbms-2002`, and `22924-constraints-2004` contain independent submissions, source files, assets, and writeups. Most work happens inside assignment subfolders like `20364-compilation-2001/16`, `20554-java-2004/15`, or `22924-constraints-2004/13/src`. Read each local `readme.md` before editing; build rules and assumptions vary by assignment.

## Build, Test, and Development Commands

There is no repo-wide build. Use commands from the relevant assignment directory:

- `gcc -c mmn13.c -DVARS_NUM=20 -DDOMAIN_SIZE=7 && gcc -o mmn13 mmn13.o` from `22924-constraints-2004/13/src`: builds the CSP solver.
- `make` from `22924-constraints-2004/13/src`: uses the historical makefile; verify the output name because the shipped file may emit `mmn13.`.
- `javac WeatherServer.java WeatherClient.java` from `20554-java-2004/15`: compiles the Java networking exercise.
- `javac TrafficLights.java ErasePic.java` from `20554-java-2004/16`: compiles the Java GUI applets.

Prefer running tools from the assignment folder they belong to so relative paths and assets resolve correctly.

## Coding Style & Naming Conventions

Preserve the style already present in each course folder. Older C/C++ code uses tabs or 4-space indentation, uppercase header names such as `GLOBAL_defs.h`, and mixed legacy naming. Java files use one public class per file with PascalCase names such as `TrafficLights.java`. Keep new files and symbols consistent with the local module, and avoid broad cleanup-only rewrites in historical code.

## Testing Guidelines

Most assignments do not include automated tests. Validate changes with the assignment’s documented workflow: compile successfully, then run a small manual scenario. Examples: run `./mmn13 -i 1 -alg fc` for the constraints solver, or execute the Java client/server pairs locally. If you add a regression harness, place it beside the assignment it covers and name it clearly, for example `test-input-small.txt` or `sample-run.sh`.

## Commit & Pull Request Guidelines

Recent history uses short, direct commit subjects, often prefixed by course numbers, for example `22918,20554` or `gen agent instructions to onboard ai - for 22924.` Follow that pattern: keep subjects concise, mention the affected course or assignment first, and describe the change plainly. Pull requests should identify the exact folder(s) changed, summarize build or manual verification steps, and include screenshots only when updating GUI or image-backed coursework.

## Agent-Specific Notes

When working as an automated contributor, treat each assignment as isolated. Do not normalize filenames, encodings, or generated artifacts across the archive unless the task explicitly requires it.
