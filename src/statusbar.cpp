#include "../include/statusbar.hpp"
#include "gtkmm/enums.h"

StatusBar::StatusBar() : Gtk::Box{Gtk::Orientation::VERTICAL} {
	m_status_label.set_text("");
	set_hexpand(true);
	set_vexpand(false);
	m_status_label.set_margin(4);
	m_status_label.set_halign(Gtk::Align::START);
	append(m_status_label);
	m_status_sep.set_hexpand(true);
	append(m_status_sep);
}

void StatusBar::set_status_text(const Glib::ustring &new_text) {
	m_status_label.set_text(new_text);
}
