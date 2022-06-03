# gst-demo-plugins

Pre-requisites:
===============
 https://gstreamer.freedesktop.org/documentation/installing/on-linux.html?gi-language=c
 

Build Commands:
===============

 meson <builddir>
 ninja -C <builddir>

Tests (uninstalled):
====================
 
 env GST_PLUGIN_PATH=build/videoeffects gst-launch-1.0 videotestsrc ! neovideoconv ! video/x-raw,width=1920,height=1440,framerate=30/1 ! videoconvert ! autovideosink

