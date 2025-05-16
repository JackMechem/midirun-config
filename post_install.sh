#!/bin/bash

echo "Applying udev rules..."

sudo udevadm control --reload-rules
sudo udevadm trigger

echo "Done"
