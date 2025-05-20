#pragma once
#include <gtkmm.h>
#include <string>
#include <thread>

class LabeledEntryBox : public Gtk::Box {
  public:
	LabeledEntryBox(const Glib::ustring &label_text, const std::string _id,
					bool labeled = false);

	void set_label(const Glib::ustring &new_text);
	void set_entry_text(const Glib::ustring &text);
	void remove_label();
	Glib::ustring get_entry_text() const;

	void set_id(std::string _id) { id = _id; }
	std::string get_id() { return id; }
	std::thread m_ListenerThread;
	std::atomic<bool> m_EntryFocused = false;

  private:
	Gtk::Label m_Label;
	Gtk::Entry m_Entry;
	std::string id;

  protected:
	bool listeningForKeys;
};
