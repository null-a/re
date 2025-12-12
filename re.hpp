#pragma once

#include <iosfwd>
#include <string_view>
#include <vector>
#include <memory>
#include <optional>
#include <variant>

struct RegExp;
struct Sym { char c; };
struct Seq { std::unique_ptr<RegExp> l, r; };
struct Alt { std::unique_ptr<RegExp> l, r; };
struct Star { std::unique_ptr<RegExp> r; };
struct RegExp { std::variant<Sym, Seq, Alt, Star> node; };

struct Symbol { char c; };
struct Jump { unsigned l; };
struct Fork { unsigned l1; unsigned l2; };
struct Label { unsigned l; };
struct Match {};

using Instr = std::variant<Symbol, Jump, Fork, Label, Match>;
using Code = std::vector<Instr>;

std::optional<RegExp> parse(std::string_view);
Code compile(const RegExp&);
bool match(const Code&, std::string_view);

enum class MatchResult { parse_error, no_match, match };
MatchResult match(std::string_view, std::string_view);

std::ostream& operator<<(std::ostream&, const RegExp&);
std::ostream& operator<<(std::ostream&, const Instr&);
std::ostream& operator<<(std::ostream&, const MatchResult&);
