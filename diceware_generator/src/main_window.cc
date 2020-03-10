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

#include "main_window.h"
#include <giomm/resource.h>
#include <giomm/simpleactiongroup.h>
#include <gtkmm/main.h>
#include <gtkmm/menubutton.h>
#include <sodium.h>

const std::string MainWindow::SPECIAL_CHARS_{"!@#$%^&*();.,"};

MainWindow::MainWindow()
    : box_container_{Gtk::ORIENTATION_VERTICAL},
      box_options_{Gtk::ORIENTATION_HORIZONTAL},
      box_buttons_{Gtk::ORIENTATION_HORIZONTAL}, chk_hyphen_{"Hyphens"},
      chk_space_{"Spaces"}, btn_generate_{"Generate"},
      chk_random_char_{"Random Character"}, btn_copy_{"Copy"},
      btn_quit_{"Quit"}, rdo_eff_{"EFF"}, rdo_diceware_{"Diceware"},
      wordlist_{Diceware::Wordlist::EFF}, num_words_{6} {
  InitializeLayout();
  InitializeSignals();
  GenerateNewWords();
  UpdateDisplay();
}

void MainWindow::InitializeLayout() {
  Gtk::MenuButton menu_button;
  auto menu = Gio::Menu::create();
  menu->append("Settings", "app.settings");

  auto action_group = Gio::SimpleActionGroup::create();

  frm_wordlists_.add(box_wordlists_);
  frm_options_.add(box_options_);
  frm_num_words_.add(box_num_words_);

  box_container_.pack_start(frm_wordlists_);
  box_container_.pack_start(frm_options_);
  box_container_.pack_start(frm_num_words_);
  box_container_.pack_start(btn_generate_);
  box_container_.pack_start(ent_password_);
  box_container_.pack_start(box_buttons_);

  box_wordlists_.pack_start(rdo_eff_);
  box_wordlists_.pack_start(rdo_diceware_);

  rdo_diceware_.join_group(rdo_eff_);

  box_options_.pack_start(chk_hyphen_);
  box_options_.pack_start(chk_space_);
  box_options_.pack_start(chk_random_char_);

  box_buttons_.pack_start(btn_copy_);
  box_buttons_.pack_start(btn_quit_);

  add(box_container_);
  set_border_width(10);
  set_default_size(600, 400);

  frm_wordlists_.set_label("Wordlists");
  frm_wordlists_.set_border_width(10);
  frm_wordlists_.set_shadow_type(Gtk::SHADOW_ETCHED_OUT);

  Gtk::RadioButton::Group group;

  for (int i = 6; i <= 10; ++i) {
    Gtk::RadioButton *rdo =
        Gtk::make_managed<Gtk::RadioButton>(group, std::to_string(i));
    rdo->signal_clicked().connect(sigc::bind<int>(
        sigc::mem_fun(*this, &MainWindow::OnChangeNumWords), i));
    box_num_words_.pack_start(*rdo);
  }

  frm_options_.set_label("Options");
  frm_options_.set_border_width(10);
  frm_options_.set_shadow_type(Gtk::SHADOW_ETCHED_OUT);

  frm_num_words_.set_label("Number of words to generate");
  frm_num_words_.set_border_width(10);
  frm_num_words_.set_shadow_type(Gtk::SHADOW_ETCHED_OUT);

  ent_password_.set_editable(false);
  ent_password_.set_alignment(0.5);
  show_all_children();
}

void MainWindow::InitializeSignals() {
  btn_quit_.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::Quit));
  btn_generate_.signal_clicked().connect(
      sigc::mem_fun(*this, &MainWindow::OnClickGenerate));

  rdo_diceware_.signal_clicked().connect(sigc::bind<Diceware::Wordlist>(
      sigc::mem_fun(*this, &MainWindow::OnSelectWordlist),
      Diceware::Wordlist::DICEWARE));

  rdo_eff_.signal_clicked().connect(sigc::bind<Diceware::Wordlist>(
      sigc::mem_fun(*this, &MainWindow::OnSelectWordlist),
      Diceware::Wordlist::EFF));

  sig_hyphen_clicked_ = chk_hyphen_.signal_clicked().connect(
      sigc::mem_fun(*this, &MainWindow::OnSelectHyphen));

  sig_space_clicked_ = chk_space_.signal_clicked().connect(
      sigc::mem_fun(*this, &MainWindow::OnSelectSpace));

  btn_copy_.signal_clicked().connect(
      sigc::mem_fun(*this, &MainWindow::OnClickCopy));

  chk_random_char_.signal_clicked().connect(
      sigc::mem_fun(*this, &MainWindow::OnSelectRandom));
}

void MainWindow::GenerateNewWords() {
  words_ = diceware_.Generate(wordlist_, num_words_);
  if (is_random_) {
    InsertSpecialChar();
  }
}

std::string MainWindow::GetPassword() {
  std::string pass = "";
  for (int i = 0; i < words_.size(); ++i) {
    pass += words_[i];
    if (is_hyphen_ && i != words_.size() - 1) {
      pass += '-';
    }

    if (is_space_ && i != words_.size() - 1) {
      pass += ' ';
    }
  }
  return pass;
}

void MainWindow::OnClickGenerate() {
  GenerateNewWords();
  UpdateDisplay();
}

void MainWindow::OnSelectWordlist(const Diceware::Wordlist wordlist) {
  wordlist_ = wordlist;
  GenerateNewWords();
  UpdateDisplay();
}

void MainWindow::OnChangeNumWords(const int num) {
  num_words_ = num;
  GenerateNewWords();
  UpdateDisplay();
}

void MainWindow::OnSelectHyphen() {
  is_hyphen_ = !is_hyphen_;
  if (chk_space_.get_active()) {
    sig_space_clicked_.block();
    chk_space_.set_active(false);
    sig_space_clicked_.unblock();
    is_space_ = false;
  }
  UpdateDisplay();
}

void MainWindow::OnSelectSpace() {
  is_space_ = !is_space_;
  if (chk_hyphen_.get_active()) {
    sig_hyphen_clicked_.block();
    chk_hyphen_.set_active(false);
    sig_hyphen_clicked_.unblock();
    is_hyphen_ = false;
  }
  UpdateDisplay();
}

void MainWindow::OnSelectRandom() {
  if (!is_random_) {
    InsertSpecialChar();
    is_random_ = true;
  } else {
    RemoveSpecialChar();
    is_random_ = false;
  }
  UpdateDisplay();
}

void MainWindow::InsertSpecialChar() {
  auto word_pos = randombytes_uniform(num_words_);
  auto selected_word = words_[word_pos];
  auto char_pos = randombytes_uniform(selected_word.size());

  auto special_char_pos = randombytes_uniform(SPECIAL_CHARS_.size());
  char special_char = SPECIAL_CHARS_[special_char_pos];

  selected_word.insert(char_pos, 1, special_char);
  words_[word_pos] = selected_word;
  random_word_position_ = std::pair<int, int>{word_pos, char_pos};
}

void MainWindow::RemoveSpecialChar() {
  auto word_pos = random_word_position_.first;
  auto char_pos = random_word_position_.second;
  auto selected_word = words_[word_pos];
  selected_word.erase(char_pos, 1);
  words_[word_pos] = selected_word;
}

void MainWindow::OnClickCopy() {
  auto ref_clipboard = Gtk::Clipboard::get();
  std::vector<Gtk::TargetEntry> targets;
  targets.push_back(Gtk::TargetEntry("UTF8_STRING"));

  ref_clipboard->set(targets,
                     sigc::mem_fun(*this, &MainWindow::OnClipboardPaste),
                     sigc::mem_fun(*this, &MainWindow::OnClipboardClear));
}

void MainWindow::OnClipboardPaste(Gtk::SelectionData &selection_data, guint) {
  selection_data.set("UTF8_STRING", GetPassword());
}

void MainWindow::OnClipboardClear() {
  // Do nothing.
}
void MainWindow::UpdateDisplay() { ent_password_.set_text(GetPassword()); }

void MainWindow::Quit() { hide(); }
