#include "../include/mainwindow.hpp"
#include "../include/getkeycode.hpp"
#include "../include/labeledentrybox.hpp"
#include "../include/midilisten.hpp"
#include "RtMidi.h"
#include "gtkmm/enums.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <toml++/toml.h>
#include <vector>

bool file_exists(const std::string &path) {
	return Gio::File::create_for_path(path)->query_exists();
}

std::optional<std::string> find_css_file() {
	std::string dev_path = "data/style.css";
	if (Gio::File::create_for_path(dev_path)->query_exists())
		return dev_path;

	auto user_path = Glib::build_filename(Glib::get_user_data_dir(),
										  "midirun-config/style.css");
	if (Gio::File::create_for_path(user_path)->query_exists())
		return user_path;

	for (const auto &dir : Glib::get_system_data_dirs()) {
		auto sys_path = Glib::build_filename(dir, "midirun-config/style.css");
		if (Gio::File::create_for_path(sys_path)->query_exists())
			return sys_path;
	}

	return std::nullopt;
}

void load_css() {
	auto css_provider = Gtk::CssProvider::create();
	if (auto path = find_css_file(); path.has_value()) {
		css_provider->load_from_path(path.value());
		Gtk::StyleContext::add_provider_for_display(
			Gdk::Display::get_default(), css_provider,
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	} else {
		std::cerr << "Could not find style.css\n";
	}
}

MainWindow::MainWindow() {
	set_title("Midirun Config");

	m_status_bar.set_status_text("Midirun Config v" + std::string(VERSION));
	set_child(m_box);
	m_listen_button.signal_clicked().connect(
		sigc::mem_fun(*this, &MainWindow::on_listen_button_toggled));

	m_listen_button.set_label("Listen");
	m_listen_button.set_hexpand(false);
	m_listen_button.set_vexpand(false);

	load_css();

	configValues = read_config_file(configPath);

	m_map_name_label.set_hexpand(true);
	m_map_name_label.set_halign(Gtk::Align::START);
	m_map_b0_label.set_hexpand(true);
	m_map_b0_label.set_halign(Gtk::Align::START);
	m_map_b1_label.set_hexpand(true);
	m_map_b1_label.set_halign(Gtk::Align::START);
	m_map_key_label.set_hexpand(true);
	m_map_key_label.set_halign(Gtk::Align::START);
	m_map_label_box.set_spacing(38);
	m_map_label_box.set_margin_start(38);
	m_map_label_box.append(m_map_name_label);
	m_map_label_box.append(m_map_b0_label);
	m_map_label_box.append(m_map_b1_label);
	m_map_label_box.append(m_map_key_label);
	m_map_group_list_main_box.append(m_map_label_box);
	m_map_group_list_main_box.append(m_map_group_list_box);
	if (configValues.size() > 0) {
		for (ConfigEntry ent : configValues) {
			MappingEntry *m_map_ent =
				Gtk::make_managed<MappingEntry>(ent, true);

			m_map_group_list_box.append(*m_map_ent);
		}
	}

	m_map_group_list_box.set_spacing(10);
	m_map_group_list_box.set_focusable(true);

	//// Listen Section
	m_box_listen_content.append(m_listen_port_drop);
	m_box_listen_content.append(m_listen_button);
	m_box_listen_content.set_hexpand(true);
	m_box_listen_content.set_margin(10);
	m_box_listen_content.set_spacing(10);
	m_box_listen_content.set_focusable(true);

	// Listen Text View
	m_listen_text_buffer = Gtk::TextBuffer::create();
	m_listen_text_view.set_buffer(m_listen_text_buffer);
	m_listen_text_view.set_editable(false); // optional
	m_listen_text_view.set_wrap_mode(Gtk::WrapMode::WORD);
	m_listen_text_view.set_margin(10);
	m_listen_text_view.set_vexpand(true); // Allow vertical expansion
	m_listen_text_view.set_can_focus(false);

	m_listen_text_view.set_margin(10);
	m_listen_text_view.set_vexpand(true);

	m_listen_text_sw.set_child(m_listen_text_view);
	m_listen_text_sw.set_policy(Gtk::PolicyType::AUTOMATIC,
								Gtk::PolicyType::AUTOMATIC);

	m_listen_text_frame.set_label("");
	auto bold_label = Gtk::make_managed<Gtk::Label>();
	bold_label->set_markup("<b>Output</b>");
	m_listen_text_frame.set_label_widget(*bold_label);
	m_listen_text_frame.set_child(m_listen_text_sw);
	m_listen_text_frame.set_margin(10);
	m_listen_text_frame.set_vexpand(true);

	//// Map Section
	m_map_button.set_label("Add New Map");
	m_apply_button.set_label("Apply");

	m_map_button.set_hexpand(true);
	m_apply_button.set_hexpand(true);

	m_map_button.signal_clicked().connect(
		sigc::mem_fun(*this, &MainWindow::on_add_map_clicked));
	m_apply_button.signal_clicked().connect(
		sigc::mem_fun(*this, &MainWindow::on_apply_clicked));

	m_map_button_box.append(m_map_button);
	m_map_button_box.append(m_apply_button);
	m_map_button_box.set_focusable(true);
	m_map_button_box.set_can_focus(true);
	m_map_button_box.set_spacing(10);

	m_box_map.set_spacing(10);
	m_box_map.set_margin(10);
	m_box_map.set_focusable(true);
	m_box_map.set_valign(Gtk::Align::START);
	m_box_map.append(m_map_group_list_main_box);
	m_box_map.append(m_map_sep);
	m_box_map.append(m_map_port_drop);
	m_box_map.append(m_map_button_box);

	//// Main Box
	m_listen_sep.set_hexpand(true);
	m_box_listen.append(m_box_listen_content);
	m_box_listen.append(m_listen_text_frame);
	m_box_map.set_vexpand(true);
	m_box_listen.set_vexpand(false);

	m_map_scrolled_window.set_policy(Gtk::PolicyType::AUTOMATIC,
									 Gtk::PolicyType::AUTOMATIC);
	m_map_scrolled_window.set_child(m_box_map);

	m_box.append(m_status_bar);
	m_box.append(m_map_scrolled_window);
	m_box.append(m_listen_sep);
	m_box.append(m_box_listen);
	m_box.set_focusable(true);
	auto click_controller = Gtk::GestureClick::create();
	click_controller->signal_pressed().connect(
		[this](int, double, double) { m_box.grab_focus(); });
	m_box.add_controller(click_controller);

	if (listening == false) {
		m_listen_text_frame.hide();
	}
}

MainWindow::~MainWindow() {}

void restartMidirun() {
	std::ifstream pid_file("/tmp/midirun.pid");
	if (!pid_file) {
		std::cerr << "Failed to open PID file.\n";
	}

	int pid;
	pid_file >> pid;
	if (pid <= 0) {
		std::cerr << "Invalid PID.\n";
	}

	if (kill(pid, SIGHUP) == 0) {
		std::cout << "Sent SIGHUP to daemon (PID " << pid << ").\n";
	} else {
		perror("kill");
	}
}

void MainWindow::on_apply_clicked() {
	m_status_bar.set_status_text("Parsing Data...");
	configValues.clear();
	configValues.shrink_to_fit();
	if (m_map_group_list_box.get_first_child()) {
		for (auto *child : m_map_group_list_box.get_children()) {

			if (auto *inner_box = dynamic_cast<Gtk::Box *>(child)) {
				ConfigEntry confEnt;
				for (auto *subchild : inner_box->get_children()) {
					// Do something with each subchild
					if (auto *labeled =
							dynamic_cast<LabeledEntryBox *>(subchild)) {
						// It's a LabeledEntryBox — you can now use its methods

						if (labeled->get_id() == "name") {
							confEnt.name = labeled->get_entry_text();
						} else if (labeled->get_id() == "b0") {
							confEnt.b0 = labeled->get_entry_text();
						} else if (labeled->get_id() == "b1") {
							confEnt.b1 = labeled->get_entry_text();
						} else if (labeled->get_id() == "keys") {
							std::stringstream ss(labeled->get_entry_text());
							std::string item;

							while (std::getline(ss, item, ',')) {
								confEnt.keys.push_back(std::stoi(item));
							}
						}
					}
				}
				configValues.push_back(confEnt);
			}
		}
		for (const auto &entry : configValues) {
			std::cout << "Name: " << entry.name << ", Byte 0: " << entry.b0
					  << ", Byte 1: " << entry.b1 << ", Keys: ";
			for (int key : entry.keys) {
				std::cout << key << ",";
			}
			std::cout << std::endl;
		}
		m_status_bar.set_status_text("Creating config file...");
		create_config_file(configValues, configPath);
		m_status_bar.set_status_text(
			"Config file written, restarting midirun...");
		restartMidirun();

	} else {
		std::cout << "no maps\n";
		m_status_bar.set_status_text("ERROR: You don't have any mappings set!");
	}
}

std::vector<ConfigEntry> MainWindow::read_config_file(const std::string &path) {
	std::vector<ConfigEntry> entries;

	try {
		toml::table tbl = toml::parse_file(path);

		int port = tbl["config"]["inputPort"].value_or<int>(-1);

		m_map_port_drop.set_active(port - 1);

		if (auto mappingArray = tbl["mapping"].as_array()) {
			for (const auto &item : *mappingArray) {
				if (!item.is_table())
					continue;
				const auto &t = *item.as_table();

				ConfigEntry entry;

				if (auto val = t["name"].value<std::string>())
					entry.name = *val;

				if (auto val = t["byte0"].value<int64_t>())
					entry.b0 = std::to_string(*val);
				if (auto val = t["byte1"].value<int64_t>())
					entry.b1 = std::to_string(*val);

				if (auto arr = t["key"].as_array()) {
					for (const auto &key : *arr) {
						if (auto keyVal = key.value<int64_t>())
							entry.keys.push_back(static_cast<int>(*keyVal));
					}
				}

				entries.push_back(entry);
			}
		} else {
			std::cerr << "`mapping` is not an array of tables.\n";
			m_status_bar.set_status_text(
				"Could not import current config, pressing "
				"apply will create a new one.");
			entries.clear();
		}
	} catch (const toml::parse_error &err) {
		std::cerr << "TOML parse error: " << err.description() << "\n";
		m_status_bar.set_status_text(
			"Error parsing config (config most likely doesn't exist), pressing "
			"apply will create a new one.");
		entries.clear();
	}

	return entries;
}

void MainWindow::create_config_file(
	const std::vector<ConfigEntry> &configValues, const std::string &filename) {
	toml::table config;

	config.insert_or_assign(
		"config", toml::table{{"inputPort",
							   m_map_port_drop.get_active_row_number() + 1}});

	for (const auto &entry : configValues) {
		toml::table mapping;
		mapping.insert("name", entry.name);
		mapping.insert("byte0", std::stoi(entry.b0));
		mapping.insert("byte1", std::stoi(entry.b1));
		toml::array key_array;
		key_array.reserve(entry.keys.size());
		for (int k : entry.keys)
			key_array.push_back(static_cast<int64_t>(k));

		mapping.insert("key", key_array);

		if (auto *arr = config["mapping"].as_array()) {
			arr->push_back(mapping);
		} else {
			config.insert("mapping", toml::array{mapping});
		}
	}

	std::ofstream file(filename);
	file << config;
	file.close();
}

void MainWindow::on_add_map_clicked() {

	MappingEntry *m_map_ent = Gtk::make_managed<MappingEntry>(true);
	m_map_group_list_box.append(*m_map_ent);
}

void MainWindow::on_listen_button_toggled() {
	listening = !listening;

	if (m_map_scrolled_window.get_parent())
		m_map_scrolled_window.unparent();
	if (m_box_listen.get_parent())
		m_box_listen.unparent();
	if (m_listen_sep.get_parent())
		m_listen_sep.unparent();
	if (m_listen_map_paned.get_parent())
		m_listen_map_paned.unparent();

	m_listen_map_paned.set_start_child(m_dummy_box1);
	m_listen_map_paned.set_end_child(m_dummy_box2);

	if (listening) {

		m_listen_button.set_label("Stop Listening");

		m_listen_map_paned.set_start_child(m_map_scrolled_window);
		m_listen_map_paned.set_end_child(m_box_listen);

		m_listen_text_frame.show();
		m_box.append(m_listen_map_paned);

		std::thread listenT(&MainWindow::midiListen, this);
		listenT.detach();

	} else {
		m_listen_button.set_label("Listen");
		m_listen_text_buffer->set_text("");
		m_listen_text_frame.hide();

		m_box.append(m_map_scrolled_window);
		m_box.append(m_listen_sep);
		m_box.append(m_box_listen);
	}
}

void MainWindow::midiListen() {

	int port = m_listen_port_drop.get_active_row_number();
	if (port == -1) {
		m_listen_text_buffer->set_text("Please Select a Device!");
	} else {

		RtMidiIn *midiin = new RtMidiIn();
		std::vector<unsigned char> message;
		int nBytes, i;
		double stamp;

		// Check ports
		unsigned int nPorts = midiin->getPortCount();
		if (nPorts == 0) {
			std::cout << "No ports available!\n";
		}
		if (nPorts < port) {
			std::cout << "Port does not exist!\n";
			std::cout << "See help page (midirun {--help|h}) for more info.";
		}

		midiin->openPort(port);

		midiin->ignoreTypes(false, false, false);

		// Periodically check input queue.
		std::cout << "Reading MIDI from port " << port << std::endl;
		while (listening) {
			stamp = midiin->getMessage(&message);
			nBytes = message.size();
			std::string out;
			for (i = 0; i < nBytes; i++) {
				out += "Byte " + std::to_string(i) + " = " +
					   std::to_string((int)message[i]) + " | ";
			}
			if (nBytes > 0) {
				out += "stamp = " + std::to_string(stamp) + "\n\n";
			}

			if (out != "") {

				{
					std::lock_guard<std::mutex> lock(midiListenTextMutex);
					midiListenPendingText = out;
				}
				Glib::signal_idle().connect_once([this]() {
					std::lock_guard<std::mutex> lock(midiListenTextMutex);
					m_listen_text_buffer->insert(m_listen_text_buffer->begin(),
												 midiListenPendingText);
				});
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				out = "";
			}
		}
		std::cout << "stopped listening";
	}
}
