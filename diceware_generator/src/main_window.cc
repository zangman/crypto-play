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
#include <filesystem>
#include <fstream>
#include <gtkmm/headerbar.h>
#include <gtkmm/main.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/separatormenuitem.h>
#include <iostream>
#include <pwd.h>
#include <sodium.h>
#include <sys/types.h>
#include <unistd.h>

const std::string MainWindow::SPECIAL_CHARS_{"!@#$%^&*();.,"};

MainWindow::MainWindow()
    : box_container_{Gtk::ORIENTATION_VERTICAL},
      box_options_{Gtk::ORIENTATION_HORIZONTAL},
      box_buttons_{Gtk::ORIENTATION_HORIZONTAL}, chk_hyphen_{"Hyphens"},
      chk_space_{"Spaces"}, btn_generate_{"Generate"},
      chk_random_char_{"Random Character"}, btn_copy_{"Copy"},
      btn_quit_{"Quit"}, rdo_eff_{"EFF"}, rdo_diceware_{"Diceware"} {

  InitializeConfigFilePath();
  CreateHeaderBar();
  LoadSettings();
  InitializeLayout();
  InitializeSignals();
  InitializeAboutDialog();
  GenerateNewWords();
  UpdateDisplay();
}

void MainWindow::CreateHeaderBar() {
  auto headerbar = Gtk::make_managed<Gtk::HeaderBar>();
  headerbar->set_title("Diceware Generator");
  auto menu_btn = Gtk::make_managed<Gtk::MenuButton>();
  auto separator = Glib::RefPtr<Gtk::MenuItem>(new Gtk::SeparatorMenuItem());

  auto menu = Gio::Menu::create();
  auto about_menu = Gio::Menu::create();
  about_menu->append("About", "win.about");
  menu->append_section(about_menu);

  auto quit_menu = Gio::Menu::create();
  quit_menu->append("Quit", "win.quit");
  menu->append_section(quit_menu);

  menu_btn->set_menu_model(menu);
  headerbar->pack_end(*menu_btn);

  set_titlebar(*headerbar);
  headerbar->show();
  menu_btn->show();

  add_action("about", sigc::mem_fun(*this, &MainWindow::ShowAboutDialog));
  add_action("quit", sigc::mem_fun(*this, &MainWindow::Quit));
}

void MainWindow::InitializeLayout() {

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

  if (settings_.wordlist == Diceware::Wordlist::EFF) {
    rdo_eff_.set_active(true);
  } else {
    rdo_diceware_.set_active(true);
  }

  box_options_.pack_start(chk_hyphen_);
  box_options_.pack_start(chk_space_);
  box_options_.pack_start(chk_random_char_);

  if (settings_.is_hyphen) {
    chk_hyphen_.set_active(true);
  }

  if (settings_.is_space) {
    chk_space_.set_active(true);
  }

  if (settings_.is_random) {
    chk_random_char_.set_active(true);
  }

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
    auto rdo = Gtk::make_managed<Gtk::RadioButton>(group, std::to_string(i));
    rdo->signal_clicked().connect(sigc::bind<int>(
        sigc::mem_fun(*this, &MainWindow::OnChangeNumWords), i));
    box_num_words_.pack_start(*rdo);
    if (settings_.num_words == i) {
      rdo->set_active(true);
    }
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

  signal_hide().connect(sigc::mem_fun(*this, &MainWindow::OnHide));
}

void MainWindow::InitializeAboutDialog() {
  about_dialog_.set_program_name("Diceware Generator");
  about_dialog_.set_version("1.0");
  about_dialog_.set_comments("A simple diceware password generator.");
  auto license_str =
      "Licensed under Apache-2.0. Refer to\n\n"
      "https://www.apache.org/licenses/LICENSE-2.0.txt\n\nfor full text.";
  about_dialog_.set_license(license_str);
  about_dialog_.set_website("https://github.com/zangman/crypto-play");
  about_dialog_.set_website_label("Github");
  about_dialog_.set_logo(
      Gdk::Pixbuf::create_from_file("../share/diceware_generator/dice.svg"));
}

void MainWindow::ShowAboutDialog() {
  about_dialog_.show();
  about_dialog_.present();
}

void MainWindow::InitializeConfigFilePath() {
  std::string homedir;
  homedir = getenv("HOME");
  if (homedir.empty()) {
    homedir = getpwuid(getuid())->pw_dir;
  }
  auto config_dir = homedir + "/.config/diceware_generator";
  std::filesystem::create_directories(config_dir);
  config_file_ = config_dir + "/settings.txt";
}

bool MainWindow::LoadSettings() {
  if (!std::filesystem::exists(config_file_)) {
    std::cerr << "Couldn't find settings file.\n";
    return false;
  }

  std::ifstream ifs{config_file_};
  if (!ifs.is_open()) {
    std::cerr << "Couldn't open settings file. Using defaults." << std::endl;
    return false;
  }

  std::string wordlist_str;
  ifs >> wordlist_str;
  settings_.wordlist = Diceware::GetEnum(wordlist_str);

  ifs >> settings_.is_hyphen;
  ifs >> settings_.is_space;
  ifs >> settings_.is_random;
  ifs >> settings_.num_words;

  ifs.close();

  return true;
}

bool MainWindow::SaveSettings() {

  std::ofstream ofs{config_file_};
  if (!ofs.is_open()) {
    std::cerr << "Couldn't save settings.\n";
    return false;
  }

  auto wordlist_str = Diceware::GetEnumString(settings_.wordlist);

  ofs << wordlist_str << std::endl;
  ofs << settings_.is_hyphen << std::endl;
  ofs << settings_.is_space << std::endl;
  ofs << settings_.is_random << std::endl;
  ofs << settings_.num_words << std::endl;

  ofs.close();
  return true;
}

void MainWindow::GenerateNewWords() {
  words_ = diceware_.Generate(settings_.wordlist, settings_.num_words);
  if (settings_.is_random) {
    InsertSpecialChar();
  }
}

std::string MainWindow::GetPassword() {
  std::string pass = "";
  for (int i = 0; i < words_.size(); ++i) {
    pass += words_[i];
    if (settings_.is_hyphen && i != words_.size() - 1) {
      pass += '-';
    }

    if (settings_.is_space && i != words_.size() - 1) {
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
  settings_.wordlist = wordlist;
  GenerateNewWords();
  UpdateDisplay();
}

void MainWindow::OnChangeNumWords(const int num) {
  settings_.num_words = num;
  GenerateNewWords();
  UpdateDisplay();
}

void MainWindow::OnSelectHyphen() {
  settings_.is_hyphen = !settings_.is_hyphen;
  if (chk_space_.get_active()) {
    sig_space_clicked_.block();
    chk_space_.set_active(false);
    sig_space_clicked_.unblock();
    settings_.is_space = false;
  }
  UpdateDisplay();
}

void MainWindow::OnSelectSpace() {
  settings_.is_space = !settings_.is_space;
  if (chk_hyphen_.get_active()) {
    sig_hyphen_clicked_.block();
    chk_hyphen_.set_active(false);
    sig_hyphen_clicked_.unblock();
    settings_.is_hyphen = false;
  }
  UpdateDisplay();
}

void MainWindow::OnSelectRandom() {
  if (settings_.is_random) {
    InsertSpecialChar();
    settings_.is_random = false;
  } else {
    RemoveSpecialChar();
    settings_.is_random = true;
  }
  UpdateDisplay();
}

void MainWindow::InsertSpecialChar() {
  auto word_pos = randombytes_uniform(settings_.num_words);
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

void MainWindow::OnHide() { SaveSettings(); }

void MainWindow::Quit() { hide(); }
