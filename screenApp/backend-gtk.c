#include <stdlib.h>
#include <stdint.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "macro.h"
#include "backend.h"
#include "lvgl.h"

//Global variable
unsigned char *fb;

//Private variables
static GtkWidget *outputImgW;
static GdkPixbuf *pixBuf;
static lv_coord_t mouse_x;
static lv_coord_t mouse_y;
static lv_indev_state_t mouse_btn = LV_INDEV_STATE_REL;
static lv_key_t last_key;
static lv_indev_state_t last_key_state;

//Functions
static void delete_event_cb(GtkWidget *widget, GdkEvent *event, void *data) {
	exit(0);
}

static gboolean keyboard_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	if (event->keyval == 'x' || event->keyval == 'X') {
		exit(0);
		return TRUE;
	}
	return FALSE;
}

void backendInit() {
	fb = (unsigned char *)malloc(WIDTH * HEIGHT * DEPTH);

	GtkWidget *mainW = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mainW), WIDTH, HEIGHT);
	gtk_window_set_resizable (GTK_WINDOW(mainW), FALSE);

	pixBuf = gdk_pixbuf_new_from_data(fb, GDK_COLORSPACE_RGB, 0, 8, WIDTH, HEIGHT, WIDTH * 3, NULL, NULL);
	outputImgW = gtk_image_new();
	GtkWidget *event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER (event_box), outputImgW);
	gtk_container_add(GTK_CONTAINER (mainW), event_box);

	gtk_widget_add_events(event_box, GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events(event_box, GDK_SCROLL_MASK);
	gtk_widget_add_events(event_box, GDK_POINTER_MOTION_MASK);
	gtk_widget_add_events(mainW, GDK_KEY_PRESS_MASK);

	g_signal_connect(mainW, "key_press_event", G_CALLBACK(keyboard_press), NULL);
	g_signal_connect(mainW, "delete-event", G_CALLBACK(delete_event_cb), NULL);

	gtk_widget_show_all(mainW);
	gtk_window_set_position(GTK_WINDOW(mainW), GTK_WIN_POS_CENTER);
}


void backendRun() {
    gtk_main_iteration_do(FALSE);
}

void backendUpdate(int x, int y, int w, int h, unsigned char *colorp) {
    gtk_image_set_from_pixbuf(GTK_IMAGE(outputImgW), pixBuf);
}
