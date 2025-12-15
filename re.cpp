#include <cctype> // isalnum
#include <utility>
#include "re.hpp"

// TODO: this parser is actually a bit too restrictive. / you might
// want to match the language {'', 'a'} (with e.g. "|a") but you can't
// currently express that i think. one way to do this might be to have
// `T -> {S}` rather than `T -> S { S }`?

// TODO: could strip the labels instruction from (relabled) code.

// TODO: perhaps do the trick of using ints and a "current gen"
// counter for flags, to avoid having to touch all flags in `reset`.

// TODO: i don't need to keep two set of flags around

// TODO: is including two labels in Fork redundant? (does one branch
// always effectively continue with the proceeding instuction?)
// removing this would have the happy side-effect of making
// instructions smaller. (8 rather than 12 bytes perhaps, which is
// more than could be saved by switching away from `variant`?)

// time python3 -c "import re; print(re.search('(a*)*b', 'aaaaaaaaaaaaaaaaaaaaaaaaa'))"
// None
// real	0m5.027s
// user	0m5.001s
// sys	0m0.019s

// time ./re "(a*)*b" "aaaaaaaaaaaaaaaaaaaaaaaaa"
// no match
// real	0m0.077s
// user	0m0.016s
// sys	0m0.059s


auto sym(const char c) -> RegExp {
  return RegExp{Sym{c}};
}
auto seq(RegExp l, RegExp r) -> RegExp {
  return RegExp{Seq{std::make_unique<RegExp>(std::move(l)), std::make_unique<RegExp>(std::move(r))}};
}
auto alt(RegExp l, RegExp r) -> RegExp {
  return RegExp{Alt{std::make_unique<RegExp>(std::move(l)), std::make_unique<RegExp>(std::move(r))}};
}
auto star(RegExp r) -> RegExp {
  return RegExp{Star{std::make_unique<RegExp>(std::move(r))}};
}

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };

std::ostream& operator<<(std::ostream& out, const RegExp& r) {
  return std::visit(overload {
    [&out](const Sym& r) -> std::ostream& {
      return out << "sym(" << r.c << ')';
    },
    [&out](const Seq& r) -> std::ostream& {
      return out << "seq(" << *r.l << ',' << *r.r << ")";
    },
    [&out](const Alt& r) -> std::ostream& {
      return out << "alt(" << *r.l << ',' << *r.r << ")";
    },
    [&out](const Star& r) -> std::ostream& {
      return out << "star(" << *r.r << ")";
    },
  }, r.node);
}

struct Parser {
  std::string_view s;

  std::optional<char> peek() {
    if (!s.empty()) {
      return s.front();
    }
    else {
      return {};
    }
  }

  void consume() {
    if (!s.empty()) {
      s.remove_prefix(1);
    }
  }

  bool expect(const char c) {
    const auto d = peek();
    if (d && *d==c) {
      consume();
      return true;
    }
    else {
      return false;
    }
  }

  std::optional<RegExp> parse() {
    auto r = parseE();
    if (r && s.empty()) {
      return r;
    }
    else {
      return {};
    }
  }

  /*
    E -> T { | T }    -- alt                              Expression ::= Sequence { "|" Sequence }
    T -> S { S }      -- seq  (by juxtaposition)          Sequence or Term
    S -> P [*]        -- star (where [.] means optional)  Factor
    P -> sym | ( E )  -- symbols or parens                Atom
    ... any alphanum char ...                             Symbol
  */

  std::optional<RegExp> parseE() {
    auto t1 = parseT();
    if (!t1) return {};
    auto t = std::move(*t1);
    while (expect('|')) {
      auto t2 = parseT();
      if (!t2) return {};
      t = alt(std::move(t), std::move(*t2));
    }
    return t;
  }

  std::optional<RegExp> parseT() {
    auto s1 = parseS();
    if (!s1) return {};
    auto res = std::move(*s1);
    while (true) {
      // parsing an S necessarily requires parsing a P, which we can
      // commit to if the next character is either a symbol or an
      // opening paren.
      const auto c = peek();
      if (!(c && (isalnum(*c) || *c == '('))) break;
      auto s2 = parseS();
      if (!s2) return {};
      res = seq(std::move(res), std::move(*s2));
    }
    return res;
  }

  std::optional<RegExp> parseS() {
    auto p = parseP();
    if (!p) return {};
    if (expect('*')) {
      return star(std::move(*p));
    }
    else {
      return p;
    }
  }

  std::optional<RegExp> parseP() {
    const auto c = peek();
    if (!c) return {};
    if (isalnum(*c)) {
      consume();
      return sym(*c);
    }
    else {
      if (!expect('(')) return {};
      auto e = parseE();
      if (!e) return {};
      if (!expect(')')) return {};
      return e;
    }
  }
};

std::optional<RegExp> parse(std::string_view s) {
  return (Parser {s}).parse();
}


// COMPILER


std::ostream& operator<<(std::ostream& out, const Instr& i) {
  std::visit(overload {
    [&out](const Symbol& i) {
      out << "  SYM " << i.c;
    },
    [&out](const Jump& i) {
      out << "  JMP " << i.l;
    },
    [&out](const Fork& i) {
      out << "  FORK " << i.l1 << " " << i.l2;
    },
    [&out](const Label& i) {
      out << i.l << ":";
    },
    [&out](const Match&) {
      out << "  MATCH";
    },
  }, i);
  return out;
}


void emit(Code& code, unsigned& label, const RegExp& r) {
  std::visit(overload {
    [&code, &label](const Sym& r) {
      code.emplace_back(Symbol{r.c});
    },
    [&code, &label](const Seq& r) {
      emit(code, label, *r.l);
      emit(code, label, *r.r);
    },
    [&code, &label](const Alt& r) {
      const auto l1 { label++ };
      const auto l2 { label++ };
      const auto l3 { label++ };
      code.emplace_back(Fork{l1, l2});
      code.emplace_back(Label{l1});
      emit(code, label, *r.l);
      code.emplace_back(Jump{l3});
      code.emplace_back(Label{l2});
      emit(code, label, *r.r);
      code.emplace_back(Label{l3});
    },
    [&code, &label](const Star& r) {
      const auto l1 { label++ };
      const auto l2 { label++ };
      const auto l3 { label++ };
      code.emplace_back(Label{l1});
      code.emplace_back(Fork{l2, l3});
      code.emplace_back(Label{l2});
      emit(code, label, *r.r);
      code.emplace_back(Jump{l1});
      code.emplace_back(Label{l3});
    },
  }, r.node);
}

// the returned vector is a map from labels to line numbers
// (implementation assumes labels start at zero and are contiguous)
std::vector<unsigned> labeltable(const Code& code) {
  // TODO: there's probably something like `count` in the stdlib?
  auto label_count { 0u };
  for (auto instr : code) {
    if (std::holds_alternative<Label>(instr)) {
      label_count++;
    }
  }
  std::vector<unsigned> table (label_count);
  for (auto line=0u; line<code.size(); ++line) {
    if (std::holds_alternative<Label>(code[line])) {
      const auto label { std::get<Label>(code[line]).l };
      table[label] = line;
    }
  }
  return table;
}

// replace labels with line numbers. for simplicity, the labels
// themselves remain in place
void relabel(Code& code) {
  const auto lookup { labeltable(code) };
  for (Instr& instr : code) {
    std::visit(overload {
      [&lookup](Jump& jmp) {
        jmp.l = lookup[jmp.l];
      },
      [&lookup](Fork& fork) {
        fork.l1 = lookup[fork.l1];
        fork.l2 = lookup[fork.l2];
      },
      [&lookup](Label& label) {
        label.l = lookup[label.l];
      },
      [](Symbol&) {
      },
      [](Match&) {
      },
    }, instr);
  }
}

Code compile(const RegExp& r) {
  Code code {};
  unsigned label {};
  emit(code, label, r);
  relabel(code);
  code.emplace_back(Match{});
  return code;
}



// VM

struct pcstack {
  std::vector<unsigned> stack;
  std::vector<bool> flags;

  pcstack(std::size_t size)
    : stack {} // could reserve capacity? (perhaps by calling `reserve` in the ctor body?)
    , flags { std::vector<bool>(size) }
  {}

  bool empty() {
    return stack.empty();
  }

  void push(const unsigned pc) {
    if (!flags.at(pc)) {
      stack.push_back(pc);
      flags[pc] = true;
    }
  }

  std::optional<unsigned> pop() {
    if (stack.empty()) return {};
    const auto val { stack.back() };
    stack.pop_back();
    return val;
  }

  // i think this is ok asymptotically, since we potentially already
  // touch every element of this at each char step. i.e. if the
  // asymptotics is O(nm), then this preserves that. (where n is
  // length of string, and m is number of instructions.) probably
  // makes constant worse though, since now we guarantee we do O(m)
  // work at ech character, even though there may be far fewer threads
  // than that in-flight.
  void reset() {
    // assume stack is already empty
    // (aside: i see noticed that this ends up using `memset`)
    flags.assign(flags.size(), false);
  }
};

bool match(const Code& code, std::string_view s) {

  pcstack threads {code.size()};
  pcstack next {code.size()};
  threads.push(0); // start with a single thread with pc=0

  // this loop is setup such that it runs once of each char in the
  // input, then one final time with `c==none`. the final iteration
  // allows us to advance threads past any remaining jumps, forks,
  // labels, etc. and potentially reach a MATCH.
  while (true) {
    const auto c { !s.empty() ? s.front() : std::optional<char>{} };

    // TODO: fast exit if we start out with zero threads?

    while (!threads.empty()) {
      const auto pc { *threads.pop() };

      if (std::visit(overload {
            [c, pc, &next](const Symbol& sym) {
              if (c && *c == sym.c) {
                next.push(pc+1);
              } // else thread dies
              return false;
            },
            [&threads](const Jump& jmp) {
              threads.push(jmp.l);
              return false;
            },
            [&threads](const Fork& fork) {
              threads.push(fork.l1);
              threads.push(fork.l2);
              return false;
            },
            [pc, &threads](const Label&) {
              threads.push(pc+1); // nop
              return false;
            },
            [c](const Match&) {
              // we found a match if we get here (the end of the
              // program) and all input is consumed. otherwise,
              // there's unconsumed input, so this thread dies.
              return !c;
            },
          }, code[pc])) return true; // exit the whole `match` function if we found a match
    }

    if (!c) break; // input fully consumed

    // afaict this will use moves rather than copies, even for
    // user-defined struct `pcstack`, so ends up being reasonably
    // efficient.
    std::swap(threads, next);
    next.reset();
    // advance along input string
    s.remove_prefix(1);
  }

  return false;
}

MatchResult match(std::string_view re, std::string_view str) {
  auto r = parse(re);
  if (!r) return MatchResult::parse_error;
  return match(compile(*r), str) ? MatchResult::match : MatchResult::no_match;
}

std::ostream& operator<<(std::ostream& out, const MatchResult& result) {
  switch (result) {
  case MatchResult::parse_error:
    out << "parse_error";
    break;
  case MatchResult::no_match:
    out << "no_match";
    break;
  case MatchResult::match:
    out << "match";
    break;
  }
  return out;
}
