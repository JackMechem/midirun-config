#!/bin/bash

echo "Applying udev rule for /dev/input/event*..."

echo "Coppying udev rule file"
sudo cp data/udev/99-midirun-config.rules /etc/udev/rules.d
echo "Reloading udev rules"
sudo udevadm control --reload-rules
sudo udevadm trigger

echo "Done"
