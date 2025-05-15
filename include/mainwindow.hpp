#pragma once

#include "../include/devicedropdown.hpp"
#include "gdkmm/device.h"
#include "gtkmm/enums.h"
#include "gtkmm/label.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/separator.h"
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
	struct ConfigEntry {
		std::string name;
		std::string b0;
		std::string b1;
		std::vector<int> keys;
	};
	struct passwd *pw = getpwuid(getuid());
	const std::string homedir = pw->pw_dir;
	std::string configPath = homedir + "/.config/midirun/config.toml";
	void on_button_toggled();

	void on_add_map_clicked();

	void on_apply_clicked();

	void create_config_file(const std::vector<ConfigEntry> &configValues,
							const std::string &filename);
	std::vector<ConfigEntry> read_config_file(const std::string &path);

	void midiListen();

	Gtk::Box m_Box{Gtk::Orientation::VERTICAL};
	Gtk::Box m_BoxTop{Gtk::Orientation::HORIZONTAL};

	Gtk::Box m_box_status{Gtk::Orientation::VERTICAL};
	Gtk::Label m_status_label;
	Gtk::Separator m_status_sep;

	Gtk::Box m_BoxMap{Gtk::Orientation::VERTICAL};
	Gtk::ScrolledWindow m_MapScrolledWindow;
	Gtk::Separator m_Sep1{Gtk::Orientation::HORIZONTAL};
	Gtk::Paned paned{Gtk::Orientation::VERTICAL};
	Gtk::Box m_BoxListen{Gtk::Orientation::VERTICAL};
	Gtk::Button m_listen_button;
	Gtk::Box m_map_button_box{Gtk::Orientation::HORIZONTAL};
	Gtk::Button m_map_button;
	Gtk::Button m_apply_button;

	Gtk::Box m_map_group_list_box{Gtk::Orientation::VERTICAL};
	Gtk::Separator m_SepMap{Gtk::Orientation::HORIZONTAL};
	DeviceDropdown m_PortDropMap{"Device"};

	Gtk::TextView m_text_view;
	Gtk::ScrolledWindow m_text_sw;
	Glib::RefPtr<Gtk::TextBuffer> m_text_buffer;
	Gtk::Frame m_text_frame;

	DeviceDropdown m_PortDrop{"Device"};

	std::string pendingText;
	std::mutex textMutex;

	Gtk::Box m_DummyBox1;
	Gtk::Box m_DummyBox2;

	std::vector<ConfigEntry> configValues;

	bool listening = false;
};
