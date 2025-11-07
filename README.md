# PWSCP (PipeWire Simple Control Panel)

A simple pipewire ASIO-like control panel


# Introduction

Before writing this software I got pretty 
frustrated about the state of pro-audio stuff on Linux: 
there are a lot of very powerful and capable software for interacting with
the patchbay (es: qpwgraph, pipecontrol, coppwr, ...), but nothing simple
enough to be a 3-click solution.

This utility aims to solve the issue and let even non-experts "get the job done".

# Requisites

To make the app look usable, you need to install the `breeze breeze5 breeze-gtk` packages.

If the app still looks bad on gnome, just know I gave up on this.

# Usage

The app has few features:
- A `File` menu at the top
- To choose Sample Rate and Buffer Size, you get a menu for each option
- 2 modes of execution per option
- 3 action buttons

When you close the app from the Close button in the top-right corner, 
the app goes in the system tray. Double-clicking the icon restores the window, 
right-clicking it you can choose either to entirely quit the app or restore the window.

It's that easy!

## Top menu bar

In the menu bar you have a `File` menu where you can find 3 actions 
triggerable via shortcuts:
- Apply
- Reload
- Quit
