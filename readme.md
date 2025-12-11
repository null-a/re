This repository contains an implementation of a bare-bones regular
expression matching routine. It's bare-bones because it only supports:

* A simple `match` routine that returns a `bool` indicating whether a
  given string matches a given regular expression.
* Strings of byte-wide characters.
* Regular expressions comprised of sequences of alphanumeric literals,
  the `|` (alternation) and `*` (repetition) operators, and
  parenthesis.

It is based on Russ Cox'
[description](https://swtch.com/~rsc/regexp/regexp2.html) of work by
Ken Thompson. A key feature of Thompson's approach is that its
run-time is linear in the length of the input string, as a result of
not using back-tracking.

I wrote this as an exercise to gain some familiarity with C++, and
it's not intended for "production" use. For example, one shortcoming
is that the parser uses recursion and will fail ungracefully when
given a sufficiently large regular expression as input.
