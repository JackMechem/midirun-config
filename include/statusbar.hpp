
#include <gtkmm.h>
class StatusBar : public Gtk::Box {
  public:
	StatusBar();
	void set_status_text(const Glib::ustring &new_text);

  private:
	Gtk::Label m_status_label;
	Gtk::Separator m_status_sep;
};
