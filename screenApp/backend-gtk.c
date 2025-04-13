#include <stdlib.h>
#include <stdint.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "macro.h"
#include "lvgl.h"

//Global variable
unsigned char *fb;

//Private variables
static unsigned char *fb_;
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
	if (event->keyval == 'x' || event->keyval == 'X') {
		exit(0);
		return TRUE;
	}

	uint32_t ascii_key = event->keyval;
	switch(event->keyval) {
	case GDK_KEY_rightarrow:
	case GDK_KEY_Right:
		ascii_key = LV_KEY_RIGHT;
		break;
	case GDK_KEY_leftarrow:
	case GDK_KEY_Left:
		ascii_key = LV_KEY_LEFT;
		break;
	case GDK_KEY_uparrow:
	case GDK_KEY_Up:
		ascii_key = LV_KEY_UP;
		break;
	case GDK_KEY_downarrow:
	case GDK_KEY_Down:
		ascii_key = LV_KEY_DOWN;
		break;
	case GDK_KEY_Escape:
		ascii_key = LV_KEY_ESC;
		break;
	case GDK_KEY_BackSpace:
		ascii_key = LV_KEY_BACKSPACE;
		break;
	case GDK_KEY_Delete:
		ascii_key = LV_KEY_DEL;
		break;
	case GDK_KEY_Tab:
		ascii_key = LV_KEY_NEXT;
		break;
	case GDK_KEY_KP_Enter:
	case GDK_KEY_Return:
	case '\r':
		ascii_key = LV_KEY_ENTER;
		break;

		default:
		break;
	}
	last_key = ascii_key;
	last_key_state = LV_INDEV_STATE_PRESSED;
	return TRUE;
}

static gboolean keyboard_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	last_key = 0;
	last_key_state = LV_INDEV_STATE_RELEASED;
	return TRUE;
}

void backendInit_(int argc, char *argv[]) {
	gtk_init(&argc, &argv);

	fb_ = (unsigned char *)malloc(WIDTH * HEIGHT * DEPTH);
	fb = (unsigned char *)malloc(WIDTH * HEIGHT * DEPTH);

	GtkWidget *mainW = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mainW), WIDTH, HEIGHT);
	gtk_window_set_resizable (GTK_WINDOW(mainW), FALSE);

	pixBuf = gdk_pixbuf_new_from_data(fb_, GDK_COLORSPACE_RGB, 0, 8, WIDTH, HEIGHT, WIDTH * 3, NULL, NULL);
	outputImgW = gtk_image_new();
	GtkWidget *event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER (event_box), outputImgW);
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


void backendRun_() {
	gtk_main_iteration_do(FALSE);
}

void backendUpdate(int x, int y, int w, int h, unsigned char *colorp) {
	for (int yy = 0; yy < h; yy++)
		memcpy(fb_ + ((yy + y) * WIDTH + x) * DEPTH, fb + (yy * w) * DEPTH, w * DEPTH);
	gtk_image_set_from_pixbuf(GTK_IMAGE(outputImgW), pixBuf);
}

static void backendPointer(lv_indev_t *indev, lv_indev_data_t *data) {
	data->point.x = mouse_x <= 8 ? 0 : mouse_x >= 136 ? 127 : (mouse_x - 8);
	data->point.y = mouse_y;
	data->state = mouse_btn;
}

lv_indev_t *backendInitPointer() {
	lv_indev_t *indevP = lv_indev_create();
	lv_indev_set_type(indevP, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(indevP, backendPointer);
	return indevP;
}

static void backendKeyboard(lv_indev_t *indev, lv_indev_data_t *data) {
	data->key = last_key;
	data->state = last_key_state;
}

lv_indev_t *backendInitKeyboard() {
	lv_indev_t *indevK = lv_indev_create();
	lv_indev_set_type(indevK, LV_INDEV_TYPE_KEYPAD);
	lv_indev_set_read_cb(indevK, backendKeyboard);
	return indevK;
}
