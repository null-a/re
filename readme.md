# `re`

## Introduction

This repository contains an implementation of a bare-bones regular
expression matching routine. It is based on Russ Cox's
[description](https://swtch.com/~rsc/regexp/regexp2.html) of work by
Ken Thompson. This implementation is bare-bones in that it only
supports:

* A simple `match` routine that returns a value indicating whether a
  given string matches a given regular expression.
* Strings of byte-wide characters.
* Regular expressions comprised of sequences of alphanumeric literals,
  the `|` (alternation) and `*` (repetition) operators, and
  parenthesis.

I wrote this as an exercise to gain some familiarity with C++, and
it's not intended for production use. For example, one limitation is
that the parser uses recursion and will fail ungracefully when given a
sufficiently large regular expression as input.

## Build

```bash
cmake -B build
cd build
make
```

## Usage

The `match` binary can be used to exercise the implementation from the
command line. For example:

```bash
./match "a*b" "aaaab"
# => match
```

## Performance

A key feature of Thompson's approach is that it doesn't use
back-tracking, leading to a worst-case run-time that is linear in the
length of the input string.
