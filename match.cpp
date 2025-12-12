#include <iostream>
#include "re.hpp"

int main(const int argc, const char* argv[]) {
  if (argc < 2) {
    std::cout << "usage: match <regex> [<str>]\n";
    return 1;
  }

  auto r = parse(argv[1]);
  if (!r) {
    std::cout << "parse failed\n";
    return 1;
  }

  // std::cout << *r << '\n';

  auto code { compile(*r) };

  if (argc >= 3) {
    // test against arg
    const auto m { match(code, argv[2]) };
    std::cout << (m ? "" : "no ") << "match\n";
  }
  else {
    // else dump code
    for (auto i : code) {
      std::cout << i << '\n';
    }
  }

}
