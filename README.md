# TreeMaker

[TreeMaker] is a computational origami tool developed by [Robert J. Lang]
for calculating crease patterns from stick figure–like tree diagrams. His talk
[*The math and magic of origami*][talk] includes a brief introduction to the
tool:

[![The math and magic of origami, Robert Lang, TED2008][talk_preview]][talk]

The software was last updated in 2006 and doesn't readily run on recent Ubuntu
releases. This repository provides a compatible legacy environment to allow it
to run on recent releases without modification.

## Dependencies

  - [Docker]
  - an [X server]

## Installation

 1. Clone the repository.

 1. Build TreeMaker and its environment:

    ```bash
    docker build --tag treemaker .
    ```

## Usage

Included is a [wrapper] for `docker run` that configures access to the X server. Read it and, if necessary, adjust it to fit your system.

Run TreeMaker via the wrapper:

```console
$ ./docker_run_x --rm treemaker --help
Usage: TreeMaker [-v] [-h] [-d <str>] [document...]
  -v, --version         show program version
  -h, --help            show option list
  -d, --datadir=<str>   TreeMaker data directory path prefix
```


  [Docker]: https://www.docker.com/
  [Robert J. Lang]: https://langorigami.com/
  [talk]: https://youtu.be/NYKcOFQCeno?t=283
  [talk_preview]: https://img.youtube.com/vi/NYKcOFQCeno/mqdefault.jpg
  [TreeMaker]: https://langorigami.com/article/treemaker/
  [wrapper]: docker_run_x
  [X server]: https://en.wikipedia.org/wiki/X_Window_System#Implementations
