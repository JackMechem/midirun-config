#include "../include/labeledentrybox.hpp"
#include "../include/getkeycode.hpp"
#include <sstream>
#include <thread>

LabeledEntryBox::LabeledEntryBox(const Glib::ustring &label_text,
								 const std::string _id)
	: Gtk::Box(Gtk::Orientation::VERTICAL) {
	id = _id;
	m_Label.set_text(label_text);
	m_Label.set_halign(Gtk::Align::START);
	m_Label.set_valign(Gtk::Align::CENTER);
	m_Entry.set_hexpand(true);

	listeningForKeys = false;

	if (_id == "keys") {
		m_Entry.set_editable(false);
		auto focus_controller = Gtk::EventControllerFocus::create();
		focus_controller->signal_enter().connect([this]() {
			m_EntryFocused = true;

			if (m_ListenerThread.joinable())
				m_ListenerThread.join();

			m_ListenerThread = std::thread([this]() {
				std::stringstream entryText;
				while (m_EntryFocused) {
					int key = GetKeycode::getKey(m_EntryFocused);
					if (m_EntryFocused == false) {
						break;
					}
					if (key == -1) { // In case permisions are not set correctly
						m_Entry.set_editable(true);
						Glib::signal_idle().connect([this]() -> bool {
							m_Entry.set_editable(true);
							return false;
						});
						return;
					}
					entryText << key;
					entryText << ",";
					Glib::signal_idle().connect(
						[text = entryText.str(), this]() -> bool {
							m_Entry.set_text(text);
							return false;
						});
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
			});
		});

		focus_controller->signal_leave().connect([this]() {
			m_EntryFocused = false;

			if (m_ListenerThread.joinable())
				m_ListenerThread.join();
		});

		m_Entry.add_controller(focus_controller);
	}

	append(m_Label);
	append(m_Entry);
}

void LabeledEntryBox::set_label(const Glib::ustring &new_text) {
	m_Label.set_text(new_text);
}

void LabeledEntryBox::set_entry_text(const Glib::ustring &text) {
	m_Entry.set_text(text);
}

Glib::ustring LabeledEntryBox::get_entry_text() const {
	return m_Entry.get_text();
}
