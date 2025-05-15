#pragma once
#include "gtkmm/comboboxtext.h"
#include <gtkmm.h>

class DeviceDropdown : public Gtk::Box {
  public:
	DeviceDropdown(const Glib::ustring &label_text);

	void set_label(const Glib::ustring &new_text);
	void set_active(const int &row) { m_ComboBox.set_active(row); }
	int get_active_row_number() const;

  private:
	Gtk::Label m_Label;
	Gtk::ComboBoxText m_ComboBox;
};
