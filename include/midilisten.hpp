#include "RtMidi.h"
#include <string>
#include <vector>

class MidiListen {
  public:
	static std::vector<std::string> getMidiOuts() {
		RtMidiIn *midiin = 0;

		std::vector<std::string> output;

		// RtMidiIn constructor
		try {
			midiin = new RtMidiIn();
		} catch (RtMidiError &error) {
			error.printMessage();
			exit(EXIT_FAILURE);
		}

		// Check inputs.
		unsigned int nPorts = midiin->getPortCount();
		std::cout << "There are " << nPorts
				  << " MIDI input sources available.\n";
		std::string portName;
		for (unsigned int i = 0; i < nPorts; i++) {
			try {
				portName = midiin->getPortName(i);
			} catch (RtMidiError &error) {
				error.printMessage();
				return std::vector<std::string>{error.getMessage()};
			}

			output.push_back(portName);
		}

		delete midiin;

		return output;
	}
};
