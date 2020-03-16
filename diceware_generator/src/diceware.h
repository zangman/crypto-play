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

#pragma once

#include <string>
#include <vector>

class Diceware {
public:
  enum class Wordlist { EFF, DICEWARE };
  static std::string GetEnumString(Wordlist enum_val);
  static Wordlist GetEnum(std::string enum_str);

  Diceware();
  std::vector<std::string> Generate(Wordlist wordlist, int num_words);

private:
  static const std::string EFF_FILE;
  static const std::string DICEWARE_FILE;

  std::vector<std::string> ReadFile(const std::string &path);
  std::vector<std::string> eff_words_;
  std::vector<std::string> diceware_words_;
};
