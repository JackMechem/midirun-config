
#include "../include/configentry.hpp"
#include "../include/labeledentrybox.hpp"
#include "gtkmm/button.h"
#include "gtkmm/object.h"
#include <gtkmm.h>
class MappingEntry : public Gtk::Box {
  public:
	MappingEntry(ConfigEntry ent, bool labeled = false);
	MappingEntry(bool labeled = false);

  private:
	LabeledEntryBox *nameEntry =
		Gtk::make_managed<LabeledEntryBox>("Name", "name");
	LabeledEntryBox *b0Entry =
		Gtk::make_managed<LabeledEntryBox>("Byte 0", "b0");
	LabeledEntryBox *b1Entry =
		Gtk::make_managed<LabeledEntryBox>("Byte 1", "b1");
	LabeledEntryBox *keyEntry =
		Gtk::make_managed<LabeledEntryBox>("Keys", "keys");
	Gtk::Button *remove_button = Gtk::make_managed<Gtk::Button>("✕");
};
