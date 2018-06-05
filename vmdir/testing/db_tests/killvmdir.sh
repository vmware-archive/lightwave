#!/bin/bash

sleep 5
pkill vmdird
if [ $? -eq 0 ]; then
	echo "Killed vmdird"
fi
