#include "../include/getkeycode.hpp"
#include <atomic>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <libevdev/libevdev.h>
#include <libudev.h>
#include <string>
#include <unistd.h>

#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <poll.h>
#include <string>
#include <unistd.h>
#include <vector>

int GetKeycode::getKey(std::atomic<bool> &keepGoing) {
	struct InputDevice {
		int fd;
		libevdev *dev;
	};

	namespace fs = std::filesystem;
	std::vector<InputDevice> keyboards;

	struct udev *udev_ctx = udev_new();
	if (!udev_ctx) {
		std::cerr << "Failed to init udev\n";
		return -1;
	}

	// Open all valid keyboard devices
	for (const auto &entry : fs::directory_iterator("/dev/input")) {
		std::string path = entry.path();
		std::string filename = entry.path().filename();

		if (filename.find("event") == std::string::npos)
			continue;

		struct udev_device *udev_dev = udev_device_new_from_subsystem_sysname(
			udev_ctx, "input", filename.c_str());
		if (!udev_dev)
			continue;

		const char *is_keyboard =
			udev_device_get_property_value(udev_dev, "ID_INPUT_KEYBOARD");
		const char *is_mouse =
			udev_device_get_property_value(udev_dev, "ID_INPUT_MOUSE");
		const char *model =
			udev_device_get_property_value(udev_dev, "ID_MODEL");

		bool skip =
			!is_keyboard || std::string(is_keyboard) != "1" ||
			(is_mouse && std::string(is_mouse) == "1") ||
			(model && std::string(model).find("Receiver") != std::string::npos);

		udev_device_unref(udev_dev);
		if (skip)
			continue;

		int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
		if (fd < 0)
			continue;

		libevdev *dev = nullptr;
		if (libevdev_new_from_fd(fd, &dev) == 0 &&
			libevdev_has_event_type(dev, EV_KEY) &&
			libevdev_has_event_code(dev, EV_KEY, KEY_A)) {

			std::cout << "Listening on: " << libevdev_get_name(dev) << " ("
					  << path << ")\n";
			keyboards.push_back({fd, dev});
		} else {
			if (dev)
				libevdev_free(dev);
			close(fd);
		}
	}

	udev_unref(udev_ctx);

	if (keyboards.empty()) {
		std::cerr << "No keyboard devices found.\n";
		return -1;
	}

	// Build poll list
	std::vector<pollfd> poll_fds;
	for (const auto &k : keyboards) {
		poll_fds.push_back({k.fd, POLLIN, 0});
	}

	// Wait for keypress
	while (keepGoing) {
		int ready = poll(poll_fds.data(), poll_fds.size(), 50);
		if (ready > 0) {
			for (size_t i = 0; i < keyboards.size(); ++i) {
				if (poll_fds[i].revents & POLLIN) {
					struct input_event ev;
					while (libevdev_next_event(keyboards[i].dev,
											   LIBEVDEV_READ_FLAG_NORMAL,
											   &ev) == 0) {
						if (ev.type == EV_KEY && ev.value == 1) {
							std::cout << "Key code: " << ev.code << std::endl;

							// Cleanup
							for (auto &k : keyboards) {
								libevdev_free(k.dev);
								close(k.fd);
							}

							return ev.code;
						}
					}
				}
			}
		}
		usleep(1000); // Prevent CPU spike
	}

	// Should never hit
	return -1;
}
