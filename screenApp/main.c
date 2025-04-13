#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "macro.h"
#ifdef GTK
#include <gtk/gtk.h>
#endif
#include "backend.h"
#include "ui.h"

//Functions
int main(int argc, char *argv[]) {
#ifdef GTK
	gtk_init(&argc, &argv);
#endif
	uiInit();

	int option;
	while ((option = getopt(argc, argv, "h")) != -1) {
		switch (option) {
		case 'h':
			PRINTF("*******************************************************\n");
			PRINTF("Usage for app [-h]\n");
			PRINTF("h:		Print this usage and exit\n");
			exit(0);
			break;
		default:
			break;
		}
	}

	uiRun();
	return 0;
}
