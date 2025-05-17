#pragma once

#include "../include/devicedropdown.hpp"
#include "../include/mappingentry.hpp"
#include "../include/statusbar.hpp"
#include <atomic>
#include <gtkmm.h>
#include <mutex>
#include <pwd.h>
#include <string>
#include <vector>

class MainWindow : public Gtk::Window {
  public:
	MainWindow();
	virtual ~MainWindow();

  protected:
	std::atomic<bool> listening{false};
	std::vector<ConfigEntry> configValues;
	struct passwd *pw = getpwuid(getuid());
	const std::string homedir = pw->pw_dir;
	std::string configPath = homedir + "/.config/midirun/config.toml";

	void on_listen_button_toggled();
	void on_add_map_clicked();
	void on_apply_clicked();
	void create_config_file(const std::vector<ConfigEntry> &configValues,
							const std::string &filename);
	std::vector<ConfigEntry> read_config_file(const std::string &path);
	void midiListen();

	// Main Box
	Gtk::Box m_box{Gtk::Orientation::VERTICAL};

	// Status Bar
	StatusBar m_status_bar;
	// Gtk::Box m_box_status{Gtk::Orientation::VERTICAL};
	// Gtk::Label m_status_label;
	// Gtk::Separator m_status_sep;

	// Mapping and Listening Section Paned
	Gtk::Paned m_listen_map_paned{Gtk::Orientation::VERTICAL};
	Gtk::Box m_dummy_box1;
	Gtk::Box m_dummy_box2;

	// Mapping Section
	Gtk::Box m_box_map{Gtk::Orientation::VERTICAL};
	Gtk::ScrolledWindow m_map_scrolled_window;
	Gtk::Box m_map_button_box{Gtk::Orientation::HORIZONTAL};
	Gtk::Button m_map_button;
	Gtk::Button m_apply_button;
	Gtk::Box m_map_group_list_box{Gtk::Orientation::VERTICAL};
	Gtk::Separator m_map_sep{Gtk::Orientation::HORIZONTAL};
	DeviceDropdown m_map_port_drop{"Device"};

	// Listen Section
	Gtk::Box m_box_listen{Gtk::Orientation::VERTICAL};
	Gtk::Box m_box_listen_content{Gtk::Orientation::HORIZONTAL};
	Gtk::Separator m_listen_sep{Gtk::Orientation::HORIZONTAL};
	Gtk::Button m_listen_button;
	DeviceDropdown m_listen_port_drop{"Device"};
	// Listen Text Output
	Gtk::TextView m_listen_text_view;
	Gtk::ScrolledWindow m_listen_text_sw;
	Glib::RefPtr<Gtk::TextBuffer> m_listen_text_buffer;
	Gtk::Frame m_listen_text_frame;
	std::string midiListenPendingText;
	std::mutex midiListenTextMutex;
};
