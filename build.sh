#!/bin/bash

#look for ems in user home
if [ -d "$HOME/tibco/ems/8.0" ]; then
	export EMS_HOME="$HOME/tibco/ems/8.0"
fi
if [ -d "$HOME/tibco/ems/8.1" ]; then
	export EMS_HOME="$HOME/tibco/ems/8.1"
fi
if [ -d "$HOME/tibco/ems/8.2" ]; then
	export EMS_HOME="$HOME/tibco/ems/8.2"
fi
# look for global instance
if [ -d "/opt/tibco/ems/8.0" ]; then
	export EMS_HOME="/opt/tibco/ems/8.0"
fi
if [ -d "/opt/tibco/ems/8.1" ]; then
	export EMS_HOME="/opt/tibco/ems/8.1"
fi
if [ -d "/opt/tibco/ems/8.2" ]; then
	export EMS_HOME="/opt/tibco/ems/8.2"
fi

export DYLD_LIBRARY_PATH=/usr/lib:$EMS_HOME/lib
node-gyp configure build