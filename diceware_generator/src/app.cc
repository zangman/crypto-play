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

#include "app.h"

Glib::RefPtr<App> App::create() {
  auto app = Glib::RefPtr<App>(new App());
  return app;
}

App::App() {}

void App::on_activate() { CreateWindow(); }

void App::on_startup() { Gtk::Application::on_startup(); }

void App::CreateWindow() {
  window_ = std::make_unique<MainWindow>();
  add_window(*window_);
  window_->show();
}
