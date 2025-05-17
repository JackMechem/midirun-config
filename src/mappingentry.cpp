#include "../include/mappingentry.hpp"

MappingEntry::MappingEntry(const ConfigEntry ent)
	: Gtk::Box{Gtk::Orientation::HORIZONTAL, 6} {

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

	remove_button->signal_clicked().connect([this]() { this->unparent(); });

	remove_button->set_vexpand(false);
	remove_button->set_valign(Gtk::Align::CENTER);
	remove_button->add_css_class("round-button");
	remove_button->add_css_class("destructive-action");
	append(*remove_button);
	remove_button->set_margin_end(20);
	append(*nameEntry);
	append(*b0Entry);
	append(*b1Entry);
	append(*keyEntry);
	set_margin(10);
	set_spacing(10);
}

MappingEntry::MappingEntry() {

	remove_button->signal_clicked().connect([this]() { this->unparent(); });
	nameEntry->set_entry_text("");
	b0Entry->set_entry_text("");
	b1Entry->set_entry_text("");
	keyEntry->set_entry_text("");

	remove_button->set_vexpand(false);
	remove_button->set_valign(Gtk::Align::CENTER);
	remove_button->add_css_class("round-button");
	remove_button->add_css_class("destructive-action");
	append(*remove_button);
	remove_button->set_margin_end(20);
	append(*nameEntry);
	append(*b0Entry);
	append(*b1Entry);
	append(*keyEntry);
	set_margin(10);
	set_spacing(10);
}
