// Lovingly borrowed from https://developer.getpebble.com/guides/pebble-apps/app-events/wakeup/
#include <pebble.h>

#define WAKEUP_REASON 0
#define PERSIST_KEY_WAKEUP_ID 42

// Min 30 minutes between notifications, max 2 hours between notifications
#define WAKEUP_MIN_INTERVAL 1800
#define WAKEUP_MAX_INTERVAL 5400

static Window *s_main_window;
static TextLayer *s_output_layer;
static WakeupId s_wakeup_id;

static void schedule_wakeup() {
	time_t wakeup_time = time(NULL) + (WAKEUP_MIN_INTERVAL + (rand() % WAKEUP_MAX_INTERVAL));
	
	// Schedule event and store wakeup ID
	s_wakeup_id = wakeup_schedule(wakeup_time, WAKEUP_REASON, true);
	persist_write_int(PERSIST_KEY_WAKEUP_ID, s_wakeup_id);
}

static void wakeup_handler(WakeupId id, int32_t reason) {
  // The app has woken!
  text_layer_set_text(s_output_layer, "You are here.");

  // Delete the ID
  persist_delete(PERSIST_KEY_WAKEUP_ID);
	
	schedule_wakeup();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  text_layer_set_text(s_output_layer, "This application periodically sends notifications in the background. There's nothing to do in the main app.");
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
	
	// If wakeup is not already scheduled
	if (!wakeup_query(s_wakeup_id, NULL)) {
		schedule_wakeup();
	}
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
}

static void init(void) {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // Subscribe to Wakeup API
  wakeup_service_subscribe(wakeup_handler);

  // Was this a wakeup launch?
  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    // The app was started by a wakeup
    WakeupId id = 0;
    int32_t reason = 0;

    // Get details and handle the wakeup
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
  }
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}