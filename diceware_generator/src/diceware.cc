// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "diceware.h"
#include <fstream>
#include <iostream>
#include <sodium.h>

const std::string Diceware::EFF_FILE =
    "../share/diceware_generator/eff_large_wordlist.txt";
const std::string Diceware::DICEWARE_FILE =
    "../share/diceware_generator/diceware.wordlist.asc";

Diceware::Diceware() {
  eff_words_ = ReadFile(EFF_FILE);
  diceware_words_ = ReadFile(DICEWARE_FILE);
}

std::string Diceware::GetEnumString(Diceware::Wordlist wordlist) {
  if (wordlist == Diceware::Wordlist::EFF) {
    return "eff";
  } else {
    return "diceware";
  }
}

Diceware::Wordlist Diceware::GetEnum(std::string enum_str) {
  if (enum_str == "eff") {
    return Diceware::Wordlist::EFF;
  } else {
    return Diceware::Wordlist::DICEWARE;
  }
}

std::vector<std::string> Diceware::ReadFile(const std::string &file_path) {
  std::vector<std::string> words;
  std::ifstream word_file(file_path);
  std::string line;

  if (word_file.is_open()) {
    std::string ignore, in_str;
    while (word_file >> ignore) {
      word_file >> in_str;
      words.push_back(in_str);
    }
  } else {
    std::cerr << "Couldn't open file.";
  }

  return words;
}

std::vector<std::string> Diceware::Generate(Diceware::Wordlist wordlist,
                                            int num_words) {
  std::vector<std::string> chosen_words;
  std::vector<std::string> *cur_list;
  if (wordlist == Diceware::Wordlist::EFF) {
    cur_list = &eff_words_;
  } else if (wordlist == Diceware::Wordlist::DICEWARE) {
    cur_list = &diceware_words_;
  } else {
    throw "Invalid wordlist.";
  }

  auto wordlist_size = cur_list->size();

  for (int i = 0; i < num_words; i++) {
    auto num = randombytes_uniform(wordlist_size);
    chosen_words.push_back(cur_list->at(num));
  }

  return chosen_words;
}
