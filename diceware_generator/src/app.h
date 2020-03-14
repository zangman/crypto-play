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

#include "main_window.h"
#include <glibmm/refptr.h>
#include <gtkmm/application.h>
#include <memory>

class App : public Gtk::Application {
public:
  static Glib::RefPtr<App> create();

protected:
  std::unique_ptr<MainWindow> window_;

  App();
  void CreateWindow();
  void on_startup() override;
  void on_activate() override;
};
