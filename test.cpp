#include <iostream>
#include <tuple>
#include <array>
#include "re.hpp"

constexpr std::array cases {
  std::make_tuple("", "", MatchResult::parse_error),
  std::make_tuple("|", "", MatchResult::parse_error),
  std::make_tuple("a|", "", MatchResult::parse_error),
  std::make_tuple("|a", "", MatchResult::parse_error),
  std::make_tuple("()", "", MatchResult::parse_error),
  std::make_tuple("a(", "", MatchResult::parse_error),
  std::make_tuple("a)", "", MatchResult::parse_error),

  std::make_tuple("a", "a", MatchResult::match),
  std::make_tuple("a", "", MatchResult::no_match),
  std::make_tuple("a", "b", MatchResult::no_match),

  std::make_tuple("ab", "ab", MatchResult::match),
  std::make_tuple("ab", "", MatchResult::no_match),
  std::make_tuple("ab", "a", MatchResult::no_match),
  std::make_tuple("ab", "b", MatchResult::no_match),

  std::make_tuple("a|b", "a", MatchResult::match),
  std::make_tuple("a|b", "b", MatchResult::match),
  std::make_tuple("a|b", "", MatchResult::no_match),
  std::make_tuple("a|b", "ab", MatchResult::no_match),

  std::make_tuple("a*", "", MatchResult::match),
  std::make_tuple("a*", "a", MatchResult::match),
  std::make_tuple("a*", "aa", MatchResult::match),
  std::make_tuple("a*", "b", MatchResult::no_match),
  std::make_tuple("a*", "ab", MatchResult::no_match),

  std::make_tuple("(a|bc)*", "", MatchResult::match),
  std::make_tuple("(a|bc)*", "a", MatchResult::match),
  std::make_tuple("(a|bc)*", "bc", MatchResult::match),

  std::make_tuple("(a|bc)*", "aa", MatchResult::match),
  std::make_tuple("(a|bc)*", "bcbc", MatchResult::match),

  std::make_tuple("(a|bc)*", "abc", MatchResult::match),
  std::make_tuple("(a|bc)*", "bca", MatchResult::match),

  std::make_tuple("(a|bc)*", "abcbc", MatchResult::match),
  std::make_tuple("(a|bc)*", "bcabc", MatchResult::match),
  std::make_tuple("(a|bc)*", "bcbca", MatchResult::match),
  std::make_tuple("(a|bc)*", "b", MatchResult::no_match),
  std::make_tuple("(a|bc)*", "c", MatchResult::no_match),
  std::make_tuple("(a|bc)*", "ab", MatchResult::no_match),
  std::make_tuple("(a|bc)*", "ac", MatchResult::no_match),
  std::make_tuple("(a|bc)*", "d", MatchResult::no_match),
};

int main() {
  for (const auto &[re, str, expected] : cases) {
    const auto actual { match(re, str) };
    const auto passed { actual == expected };
    std::cout << (passed ? "PASS" : "FAIL");
    std::cout << " re='" << re << "' str='" << str << "' expected=" << expected;
    if (!passed) {
      std::cout << " actual=" << actual;
    }
    std::cout << '\n';
  }
}
