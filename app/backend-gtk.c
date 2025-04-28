#include <stdlib.h>
#include <stdint.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "macro.h"
#include "lvgl.h"
#include "backend.h"

//Global variable
unsigned char *fbPublic;

//Private variables
static unsigned char *fbPrivate;
static GdkPixbuf *pixPrivate;
static GdkPixbuf *pixPrivateScaled;
static GtkWidget *darea;
static lv_coord_t mouse_x;
static lv_coord_t mouse_y;
static lv_indev_state_t mouse_btn = LV_INDEV_STATE_REL;

//Functions
static void delete_event_cb(GtkWidget *widget, GdkEvent *event, void *data) {
	processInput('x');
}

static gboolean mouse_pressed(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	mouse_btn = LV_INDEV_STATE_PRESSED;
	return FALSE;
}

static gboolean mouse_released(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	mouse_btn = LV_INDEV_STATE_RELEASED;
	return FALSE;
}

static gboolean mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
	mouse_x = event->x;
	mouse_y = event->y;
	return FALSE;
}

static gboolean keyboard_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	return TRUE;
}

static gboolean keyboard_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	uint32_t ascii_key = event->keyval;
	switch(event->keyval) {
	case GDK_KEY_rightarrow:
	case GDK_KEY_Right:
		ascii_key = 67;
		break;
	case GDK_KEY_leftarrow:
	case GDK_KEY_Left:
		ascii_key = 68;
		break;
	case GDK_KEY_uparrow:
	case GDK_KEY_Up:
		ascii_key = 65;
		break;
	case GDK_KEY_downarrow:
	case GDK_KEY_Down:
		ascii_key = 66;
		break;
	default:
		break;
	}
	processInput(ascii_key);
	return TRUE;
}

static void pixmap_destroy_notify(guchar *pixels, gpointer data) {
}

#define FACTOR 3
static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
	gdk_pixbuf_scale(pixPrivate, pixPrivateScaled, 0, 0, FACTOR * WIDTH, FACTOR * HEIGHT, 0, 0, FACTOR, FACTOR, GDK_INTERP_BILINEAR);
	gdk_cairo_set_source_pixbuf(cr, pixPrivateScaled, 0, 0);
	cairo_paint (cr);
	return FALSE;
}

void backendInit_(int argc, char *argv[]) {
	gtk_init(&argc, &argv);

	fbPublic = (unsigned char *)malloc(WIDTH * HEIGHT * DEPTH);
	fbPrivate = (unsigned char *)malloc(WIDTH * HEIGHT * DEPTH);
	pixPrivate = gdk_pixbuf_new_from_data(fbPrivate, GDK_COLORSPACE_RGB, 0, 8, WIDTH, HEIGHT, WIDTH * DEPTH, pixmap_destroy_notify, NULL);
	pixPrivateScaled = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, FACTOR * WIDTH, FACTOR * HEIGHT);

	GtkWidget *mainW = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mainW), FACTOR * WIDTH, FACTOR * HEIGHT);
	gtk_window_set_resizable (GTK_WINDOW(mainW), FALSE);

	darea = gtk_drawing_area_new();
	gtk_widget_set_size_request(darea, WIDTH, HEIGHT);
	g_signal_connect(darea, "draw", G_CALLBACK(draw_cb), NULL);
	GtkWidget *event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER (event_box), darea);
	gtk_container_add(GTK_CONTAINER (mainW), event_box);

	gtk_widget_add_events(event_box, GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events(event_box, GDK_SCROLL_MASK);
	gtk_widget_add_events(event_box, GDK_POINTER_MOTION_MASK);
	gtk_widget_add_events(mainW, GDK_KEY_PRESS_MASK);

	g_signal_connect(event_box, "button-press-event", G_CALLBACK(mouse_pressed), NULL);
	g_signal_connect(event_box, "button-release-event", G_CALLBACK(mouse_released), NULL);
	g_signal_connect(event_box, "motion-notify-event", G_CALLBACK(mouse_motion), NULL);
	g_signal_connect(mainW, "key_press_event", G_CALLBACK(keyboard_press), NULL);
	g_signal_connect(mainW, "key_release_event", G_CALLBACK(keyboard_release), NULL);
	g_signal_connect(mainW, "delete-event", G_CALLBACK(delete_event_cb), NULL);

	gtk_widget_show_all(mainW);
	gtk_window_set_position(GTK_WINDOW(mainW), GTK_WIN_POS_CENTER);
}

void backendRotate_() {}

static void *backendWork_t(void *arg) {
	backendWork();
	return 0;
}

void backendLoop_() {
	pthread_t pth;
	pthread_create(&pth, NULL, backendWork_t, NULL);
	gtk_main();
}

void backendUpdate_(int x, int y, int w, int h, unsigned char *colorp) {
	if (rotationCur == 0) {
		for (int yy = 0; yy < h; yy++)
			for (int xx = 0; xx < w; xx++) {
				fbPrivate[((yy + y) * WIDTH + xx + x) * DEPTH + 0] = fbPublic[(yy * w + xx) * DEPTH + 2];
				fbPrivate[((yy + y) * WIDTH + xx + x) * DEPTH + 1] = fbPublic[(yy * w + xx) * DEPTH + 1];
				fbPrivate[((yy + y) * WIDTH + xx + x) * DEPTH + 2] = fbPublic[(yy * w + xx) * DEPTH + 0];
			}
	} else if (rotationCur == 1) {
		for (int yy = 0; yy < h; yy++)
			for (int xx = 0; xx < w; xx++) {
				fbPrivate[((HEIGHT - xx + x) * WIDTH + yy + y) * DEPTH + 0] = fbPublic[(yy * w + xx) * DEPTH + 2];
				fbPrivate[((HEIGHT - xx + x) * WIDTH + yy + y) * DEPTH + 1] = fbPublic[(yy * w + xx) * DEPTH + 1];
				fbPrivate[((HEIGHT - xx + x) * WIDTH + yy + y) * DEPTH + 2] = fbPublic[(yy * w + xx) * DEPTH + 0];
			}
	} else if (rotationCur == 2) {
		for (int yy = 0; yy < h; yy++)
			for (int xx = 0; xx < w; xx++) {
				fbPrivate[((HEIGHT - yy + y) * WIDTH + WIDTH - xx + x) * DEPTH + 0] = fbPublic[(yy * w + xx) * DEPTH + 2];
				fbPrivate[((HEIGHT - yy + y) * WIDTH + WIDTH - xx + x) * DEPTH + 1] = fbPublic[(yy * w + xx) * DEPTH + 1];
				fbPrivate[((HEIGHT - yy + y) * WIDTH + WIDTH - xx + x) * DEPTH + 2] = fbPublic[(yy * w + xx) * DEPTH + 0];
			}
	} else if (rotationCur == 3) {
		for (int yy = 0; yy < h; yy++)
			for (int xx = 0; xx < w; xx++) {
				fbPrivate[((xx + x) * WIDTH + WIDTH - yy + y) * DEPTH + 0] = fbPublic[(yy * w + xx) * DEPTH + 2];
				fbPrivate[((xx + x) * WIDTH + WIDTH - yy + y) * DEPTH + 1] = fbPublic[(yy * w + xx) * DEPTH + 1];
				fbPrivate[((xx + x) * WIDTH + WIDTH - yy + y) * DEPTH + 2] = fbPublic[(yy * w + xx) * DEPTH + 0];
			}
	}
#ifdef DEBUG_REDRAW
	if (rotationCur == 0) {
		for (int yyy = 0; yyy < h; yyy++) {
			fbPrivate[((yyy + y) * WIDTH + x) * DEPTH + 0] = 255;
			fbPrivate[((yyy + y) * WIDTH + x) * DEPTH + 1] = 255;
			fbPrivate[((yyy + y) * WIDTH + x) * DEPTH + 2] = 255;
			fbPrivate[((yyy + y) * WIDTH + x + w) * DEPTH + 0] = 255;
			fbPrivate[((yyy + y) * WIDTH + x + w) * DEPTH + 1] = 255;
			fbPrivate[((yyy + y) * WIDTH + x + w) * DEPTH + 2] = 255;
		}
		for (int xxx = 0; xxx < w; xxx++) {
			fbPrivate[(y * WIDTH + x + xxx) * DEPTH + 0] = 255;
			fbPrivate[(y * WIDTH + x + xxx) * DEPTH + 1] = 255;
			fbPrivate[(y * WIDTH + x + xxx) * DEPTH + 2] = 255;
			fbPrivate[((y + h) * WIDTH + x + xxx) * DEPTH + 0] = 255;
			fbPrivate[((y + h) * WIDTH + x + xxx) * DEPTH + 1] = 255;
			fbPrivate[((y + h) * WIDTH + x + xxx) * DEPTH + 2] = 255;
		}
	}
#endif
	if (darea)
		gtk_widget_queue_draw(darea);
}

void backendUninit_() {
	exit(0);
}

static void backendPointer(lv_indev_t *indev, lv_indev_data_t *data) {
	data->point.x = mouse_x / FACTOR;
	data->point.y = mouse_y / FACTOR;
	data->state = mouse_btn;
}

void backendInitPointer() {
	lv_indev_t *indevP = lv_indev_create();
	lv_indev_set_type(indevP, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(indevP, backendPointer);
}
