#include "../include/mainwindow.hpp"
#include "../include/getkeycode.hpp"
#include "../include/labeledentrybox.hpp"
#include "../include/midilisten.hpp"
#include "RtMidi.h"
#include "gtkmm/enums.h"
#include "gtkmm/object.h"
#include "sigc++/functors/mem_fun.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <toml++/toml.h>
#include <vector>

MainWindow::MainWindow() {
	set_title("Midirun Config");

	m_status_label.set_text("Welcome to midirun config");
	set_child(m_Box);
	m_listen_button.signal_clicked().connect(
		sigc::mem_fun(*this, &MainWindow::on_button_toggled));

	m_listen_button.set_label("Listen");
	m_listen_button.set_hexpand(false);
	m_listen_button.set_vexpand(false);

	auto css = Gtk::CssProvider::create();

	css->load_from_data(R"(
    .round-button {
        min-width: 34px;
        min-height: 34px;
        padding: 0;
        border-radius: 90px;
        transition-duration: 300ms;
        font-size: 18px;
    }
    )");

	Gtk::StyleContext::add_provider_for_display(
		Gdk::Display::get_default(), css, GTK_STYLE_PROVIDER_PRIORITY_USER);

	configValues = read_config_file(configPath);

	if (configValues.size() > 0) {
		for (ConfigEntry ent : configValues) {

			auto *group_box =
				Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 6);
			auto *nameEntry =
				Gtk::make_managed<LabeledEntryBox>("Name", "name");
			auto *b0Entry = Gtk::make_managed<LabeledEntryBox>("Byte 0", "b0");
			auto *b1Entry = Gtk::make_managed<LabeledEntryBox>("Byte 1", "b1");
			auto *keyEntry = Gtk::make_managed<LabeledEntryBox>("Keys", "keys");
			auto *remove_button = Gtk::make_managed<Gtk::Button>("✕");

			nameEntry->set_entry_text(ent.name);
			b0Entry->set_entry_text(ent.b0);
			b1Entry->set_entry_text(ent.b1);

			std::stringstream keysStr;

			for (int i = 0; i < ent.keys.size(); i++) {
				int key = ent.keys[i];
				keysStr << key;
				if (i + 1 != ent.keys.size())
					keysStr << ",";
			}
			keyEntry->set_entry_text(keysStr.str());

			// Connect the "Remove" button
			remove_button->signal_clicked().connect([this, group_box]() {
				m_map_group_list_box.remove(
					*group_box); // remove the whole group row
			});

			remove_button->set_vexpand(false);
			remove_button->set_valign(Gtk::Align::CENTER);
			remove_button->add_css_class("round-button");
			remove_button->add_css_class("destructive-action");
			group_box->append(*remove_button);
			remove_button->set_margin_end(20);
			group_box->append(*nameEntry);
			group_box->append(*b0Entry);
			group_box->append(*b1Entry);
			group_box->append(*keyEntry);
			group_box->set_margin(10);

			m_map_group_list_box.append(*group_box);
			m_map_group_list_box.set_spacing(10);
			m_map_group_list_box.set_margin_start(20);
			m_map_group_list_box.set_margin_end(20);
			m_map_group_list_box.set_margin_top(20);
			m_map_group_list_box.set_margin_bottom(20);
			m_map_group_list_box.set_focusable(true);
			group_box->show();
		}
	}

	// Port Dropdown
	// m_PortELabel.set_markup("<b>Device:</b>");
	// m_PortELabel.set_halign(Gtk::Align::START);
	// m_PortELabel.set_margin_bottom(5);
	// m_PortEBox.append(m_PortELabel);
	// m_PortEBox.append(m_PortDrop);
	// m_PortEBox.set_margin_end(10);
	// m_PortEBox.set_hexpand(true);

	// Top Box
	m_BoxTop.append(m_PortDrop);
	m_BoxTop.append(m_listen_button);
	m_BoxTop.set_hexpand(true);
	m_BoxTop.set_margin(10);
	m_BoxTop.set_spacing(10);
	m_BoxTop.set_focusable(true);

	// Text View
	m_text_buffer = Gtk::TextBuffer::create();
	m_text_view.set_buffer(m_text_buffer);
	m_text_view.set_editable(false); // optional
	m_text_view.set_wrap_mode(Gtk::WrapMode::WORD);
	m_text_view.set_margin(10);
	m_text_view.set_vexpand(true); // Allow vertical expansion
	m_text_view.set_can_focus(false);

	m_text_view.set_margin(10);
	m_text_view.set_vexpand(true);

	m_text_sw.set_child(m_text_view);
	m_text_sw.set_policy(Gtk::PolicyType::AUTOMATIC,
						 Gtk::PolicyType::AUTOMATIC);

	m_text_frame.set_label("");
	auto bold_label = Gtk::make_managed<Gtk::Label>();
	bold_label->set_markup("<b>Output</b>");
	m_text_frame.set_label_widget(*bold_label);
	m_text_frame.set_child(m_text_sw);
	m_text_frame.set_margin(10);
	m_text_frame.set_vexpand(true);

	// Map Section
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

	m_BoxMap.set_spacing(10);
	m_BoxMap.set_margin(10);
	m_BoxMap.set_focusable(true);
	m_BoxMap.set_valign(Gtk::Align::START);
	m_BoxMap.append(m_map_group_list_box);
	m_BoxMap.append(m_SepMap);
	m_BoxMap.append(m_PortDropMap);
	m_BoxMap.append(m_map_button_box);

	// Status Bar
	m_box_status.set_hexpand(true);
	m_box_status.set_vexpand(false);
	m_status_label.set_margin(4);
	m_status_label.set_halign(Gtk::Align::START);
	m_box_status.append(m_status_label);
	m_status_sep.set_hexpand(true);
	m_box_status.append(m_status_sep);

	// Main Box
	m_Sep1.set_hexpand(true);
	m_BoxListen.append(m_BoxTop);
	m_BoxListen.append(m_text_frame);
	m_BoxMap.set_vexpand(true);
	m_BoxListen.set_vexpand(false);

	m_MapScrolledWindow.set_policy(Gtk::PolicyType::AUTOMATIC,
								   Gtk::PolicyType::AUTOMATIC);
	m_MapScrolledWindow.set_child(m_BoxMap);

	m_Box.append(m_box_status);
	m_Box.append(m_MapScrolledWindow);
	m_Box.append(m_Sep1);
	m_Box.append(m_BoxListen);
	m_Box.set_focusable(true);
	auto click_controller = Gtk::GestureClick::create();
	click_controller->signal_pressed().connect(
		[this](int, double, double) { m_Box.grab_focus(); });
	m_Box.add_controller(click_controller);

	if (listening == false) {
		m_text_frame.hide();
	}
}

MainWindow::~MainWindow() {}

void MainWindow::on_apply_clicked() {
	m_status_label.set_text("Parsing Data...");
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
		m_status_label.set_text("Creating config file...");
		create_config_file(configValues, configPath);
		m_status_label.set_text(
			"Config file written, please restart the midirun service.");
	} else {
		std::cout << "no maps\n";
		m_status_label.set_text("ERROR: You don't have any mappings set!");
	}
}

std::vector<MainWindow::ConfigEntry>
MainWindow::read_config_file(const std::string &path) {
	std::vector<ConfigEntry> entries;

	try {
		toml::table tbl = toml::parse_file(path);

		int port = tbl["config"]["inputPort"].value_or<int>(-1);

		m_PortDropMap.set_active(port - 1);

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
			m_status_label.set_text("Could not import current config, pressing "
									"apply will create a new one.");
		}
	} catch (const toml::parse_error &err) {
		std::cerr << "TOML parse error: " << err.description() << "\n";
		m_status_label.set_text(
			"Error parsing config (config most likely doesn't exist), pressing "
			"apply will create a new one.");
	}

	return entries;
}

void MainWindow::create_config_file(
	const std::vector<ConfigEntry> &configValues, const std::string &filename) {
	toml::table config;

	config.insert_or_assign(
		"config",
		toml::table{{"inputPort", m_PortDropMap.get_active_row_number() + 1}});

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
	auto *group_box =
		Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 6);
	auto *nameEntry = Gtk::make_managed<LabeledEntryBox>("Name", "name");
	auto *b0Entry = Gtk::make_managed<LabeledEntryBox>("Byte 0", "b0");
	auto *b1Entry = Gtk::make_managed<LabeledEntryBox>("Byte 1", "b1");
	auto *keyEntry = Gtk::make_managed<LabeledEntryBox>("Keys", "keys");
	auto *remove_button = Gtk::make_managed<Gtk::Button>("✕");

	// Connect the "Remove" button
	remove_button->signal_clicked().connect([this, group_box]() {
		m_map_group_list_box.remove(*group_box); // remove the whole group row
	});

	remove_button->set_vexpand(false);
	remove_button->set_valign(Gtk::Align::CENTER);
	remove_button->add_css_class("round-button");
	remove_button->add_css_class("destructive-action");
	group_box->append(*remove_button);
	remove_button->set_margin_end(20);
	group_box->append(*nameEntry);
	group_box->append(*b0Entry);
	group_box->append(*b1Entry);
	group_box->append(*keyEntry);
	group_box->set_margin(10);

	m_map_group_list_box.append(*group_box);
	m_map_group_list_box.set_spacing(10);
	m_map_group_list_box.set_margin_start(20);
	m_map_group_list_box.set_margin_end(20);
	m_map_group_list_box.set_margin_top(20);
	m_map_group_list_box.set_margin_bottom(20);
	group_box->show();
}

void MainWindow::on_button_toggled() {
	listening = !listening;

	if (m_MapScrolledWindow.get_parent())
		m_MapScrolledWindow.unparent();
	if (m_BoxListen.get_parent())
		m_BoxListen.unparent();
	if (m_Sep1.get_parent())
		m_Sep1.unparent();
	if (paned.get_parent())
		paned.unparent();

	paned.set_start_child(m_DummyBox1);
	paned.set_end_child(m_DummyBox2);

	if (listening) {

		m_listen_button.set_label("Stop Listening");

		paned.set_start_child(m_MapScrolledWindow);
		paned.set_end_child(m_BoxListen);

		m_text_frame.show();
		m_Box.append(paned);

		std::thread listenT(&MainWindow::midiListen, this);
		listenT.detach();

	} else {
		m_listen_button.set_label("Listen");
		m_text_buffer->set_text("");
		m_text_frame.hide();

		m_Box.append(m_MapScrolledWindow);
		m_Box.append(m_Sep1);
		m_Box.append(m_BoxListen);
	}
}

void MainWindow::midiListen() {

	int port = m_PortDrop.get_active_row_number();
	if (port == -1) {
		m_text_buffer->set_text("Please Select a Device!");
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
		std::cout << "Reading MIDI from port " << port
				  << " -- quit with Ctrl-C.\n";
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
					std::lock_guard<std::mutex> lock(textMutex);
					pendingText = out;
				}
				Glib::signal_idle().connect_once([this]() {
					std::lock_guard<std::mutex> lock(textMutex);
					m_text_buffer->insert(m_text_buffer->begin(), pendingText);
				});
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				out = "";
			}
		}
		std::cout << "stopped listening";
	}
}
