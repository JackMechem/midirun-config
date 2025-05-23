#include "include/mainwindow.hpp"
#include <gtkmm/application.h>
int main(int argc, char **argv) {

	auto app = Gtk::Application::create("com.github.JackMechem.MidirunConfig");

	return app->make_window_and_run<MainWindow>(argc, argv);
}
