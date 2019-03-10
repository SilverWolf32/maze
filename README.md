# Maze

A 3D maze program, written in C. Heavily inspired by the IOCCC winner 'buzzard', though of course it's a lot bigger.

## Features

- 3D maze navigation
- Read maze from a file or pipe to stdin
- Bring up a full map of the maze at any time [ ? | ⇧/ ]
- Compass display indicates which way you're pointing [ ⇧C ]

## Usage examples

- `maze somefile.txt`
- `procedural-maze 4 | maze -`

## The maze file format

Walls are anything except spaces (usually `#`), the entrance is `[`, and the exit is `]`.

## License

Public domain, with a fallback license (the Unlicense). See [COPYING.md](./COPYING.md) for details.
