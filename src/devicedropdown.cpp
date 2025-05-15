#include "../include/devicedropdown.hpp"
#include "../include/midilisten.hpp"

DeviceDropdown::DeviceDropdown(const Glib::ustring &label_text)
	: Gtk::Box(Gtk::Orientation::VERTICAL) {
	m_Label.set_text(label_text);
	m_Label.set_halign(Gtk::Align::START);
	m_Label.set_valign(Gtk::Align::CENTER);
	m_ComboBox.set_hexpand(true);

	std::vector<std::string> ports = MidiListen::getMidiOuts();

	for (std::string port : ports) {
		m_ComboBox.append(port);
	}

	append(m_Label);
	append(m_ComboBox);
}

void DeviceDropdown::set_label(const Glib::ustring &new_text) {
	m_Label.set_text(new_text);
}

int DeviceDropdown::get_active_row_number() const {
	return m_ComboBox.get_active_row_number();
}
