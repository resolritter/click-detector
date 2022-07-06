# Introduction

This project produces a `click-detector` executable which is able to detect
mouse clicks on a Linux system. The application runs until a click is detected
and, when that happens, it prints the mouse event's data to stdout and exits.

# Building

You'll need a recent `gcc` toolchain with the standard libc headers (check out
`./main.c` to know which ones are used precisely). **Optional** X-related
debugging information is available but for that you'll also need `libx11`.

Run `./build`, or `./build x11` for the additional Xorg-related debugging info,
which will generate the program's entrypoint at `./click-detector`.

If you want to run `./click-detector` without root, your user needs to be added
to the `input` group and after that you'll need to restart your login session
(can be done by restarting the computer).

```bash
sudo usermod -a -G input "$USER"
```
