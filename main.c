#ifdef CLICK_DETECTOR_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
  Reference material:
  https://askubuntu.com/a/1044153
  https://github.com/irontec/node-mouse/blob/05475133d67665c5fd7938a1d8b85f085565cd14/mouse.js
  https://stackoverflow.com/a/67631433
*/

void print_help(char** argv) {
	printf(
		"Usage: %s\n%s\n",
		argv[0],
		"  Upon start, this program will run until a click is detected. When a "
		"click is detected it'll dump the event data and exit successfully."
	);
}

int main(int argc, char** argv) {
	if (argc == 2) {
		char* arg = argv[1];
		assert(arg);
		print_help(argv);
		if (strncmp(arg, "--help", strlen(arg)) == 0) {
			exit(0);
		} else {
			fprintf(stderr, "ERROR: Invalid argument %s\n", arg);
			exit(EXIT_FAILURE);
		}
	}

#ifdef CLICK_DETECTOR_X11
	Display* display = XOpenDisplay(0);
	Window root_window = DefaultRootWindow(display);
#endif

	/*
	  To read this device $USER needs to be root or needs to be added to the to
	  the `input` group.
	*/
	const char* event_device = "/dev/input/mice";
	int event_fd = open(event_device, O_RDONLY);
	if (event_fd == -1) {
		perror("open(event_device, O_RDONLY)");
		printf("Failed to open %s", event_device);
		return EXIT_FAILURE;
	}

	int bytes_read = 0;
	char ev[3];
	while (true) {
		bytes_read += read(event_fd, &ev, sizeof(ev) - bytes_read);
		if (bytes_read == sizeof(ev)) {
			bytes_read = 0;

			printf("ev: b0 %d b1 %d b2 %d\n", ev[0], ev[1], ev[2]);

			/*
			  https://github.com/irontec/node-mouse/blob/05475133d67665c5fd7938a1d8b85f085565cd14/mouse.js
			  and other sources in the wild demonstrate that clicks should be
			  detected by the bits of ev[0], but from practice we found out that was
			  leading to false positives from arbitrary cursor movement. So instead
			  we use a different heuristic: if an event was triggered while
			  (ev[1] == 0 && ev[2] == 0), which are used for movement-related deltas,
			  infer that no movement was registered for this event, which means it
			  was a click.
			*/
			if (ev[1] == 0 && ev[2] == 0) {
				// got some mouse event without movement (b1 and b2 unchanged),
				// interpret as a click
				printf("got click, exiting\n%d %d %d", ev[0], ev[1], ev[2]);
				break;
			}

#ifdef CLICK_DETECTOR_X11
			XEvent event;
			XQueryPointer(
				display,
				root_window,
				&event.xbutton.root,
				&event.xbutton.subwindow,
				&event.xbutton.x_root,
				&event.xbutton.y_root,
				&event.xbutton.x,
				&event.xbutton.y,
				&event.xbutton.state
			);
			printf("x=%d, y=%d\n", event.xmotion.x, event.xmotion.y);
#endif
		}
	}

	close(event_fd);
}
