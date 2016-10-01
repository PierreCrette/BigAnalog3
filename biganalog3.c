#include <pebble.h>

static int debug=1; // debug=0: No log, 1:logs, 2:verbose, 3:verbose+specific
static int printscreen=1; //0:normal 1:shows 10h10 2:shows 10h15
static int hi=0; //0:normal 1:date on the 12
static int colorscheme=1; //colorscheme change based on movments
static int handscheme=0; //handscheme change based on movments

static int exterior; //size of the exterior circle
static int count=0; //>0:show second hand 0:not
static int countmax=10; //nb of seconds to show second hand after shake
static int secondhand=0; //0 if now draw, 1 if draw
static int btvibe=0; //1 will vibe for bt loss, 0 will not vibe

static int hourhandbase, hourhandwidth, hourlenght;
static int minutehandbase, minutehandwidth, minutelenght, maxminutelenght;
static int second_hand_length;
static int pointsize, traitsize;

static GColor colorbg, colorticks, colorpoints;
static GColor colorhour, colorminute;
static GColor colortext, colorbgtext;

static Window *window;

static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_day_label, *s_num_label, *s_numhi_label;
static int s_battery_level;

static GPath *s_minute_arrow, *s_hour_arrow, *s_triangle;
static char s_num_buffer[4], s_numhi_buffer[4], s_day_buffer[6];
static int hourangle, minuteangle;

static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;

#include "biganalog3.h"

static void date_update_proc2() {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	strftime(s_day_buffer, sizeof(s_day_buffer), "%a", t);
	if ((minuteangle>43) & (minuteangle<47)) {s_day_buffer[0]='\0';}
	if (debug==3) {text_layer_set_text(s_day_label, "Wed");}
	else {text_layer_set_text(s_day_label, s_day_buffer);}

	strftime(s_num_buffer, sizeof(s_num_buffer), "%d", t);
	if (s_num_buffer[0]=='0') {
		s_num_buffer[0]=s_num_buffer[1];
		s_num_buffer[1]='\0';
		}
	strftime(s_numhi_buffer, sizeof(s_numhi_buffer), "%d", t);
	if (s_numhi_buffer[0]=='0') {
		s_numhi_buffer[0]=s_numhi_buffer[1];
		s_numhi_buffer[1]='\0';
	}
	if (debug==2) {APP_LOG(APP_LOG_LEVEL_INFO, "hourangle=%i", hourangle);}

	if (debug==3) {
		hi = 1-hi;
	}
	else {
		if ((minuteangle>12) & (minuteangle<18)) {
			hi = 1;
		} else {
			hi = 0;
		}
	text_layer_set_text(s_num_label, s_num_buffer);
	text_layer_set_text(s_numhi_label, s_numhi_buffer);

	text_layer_set_text(s_day_label, s_day_buffer);
	text_layer_set_background_color(s_day_label, colorbgtext);
	text_layer_set_text_color(s_day_label, colortext);
	
	text_layer_set_text(s_num_label, s_num_buffer);
	text_layer_set_background_color(s_num_label, colorbgtext);
	text_layer_set_text_color(s_num_label, colortext);
	
	text_layer_set_text(s_numhi_label, s_numhi_buffer);
	text_layer_set_background_color(s_numhi_label, colorbgtext);
	text_layer_set_text_color(s_numhi_label, colortext);
	}
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
  if(!connected) {
    // Issue a vibrating alert
    if (btvibe > 0) {vibes_double_pulse();}
  }
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
	if (debug==2) {
		uint16_t ms = time_ms(NULL,NULL);		
		APP_LOG(APP_LOG_LEVEL_INFO, " Begin bg_update_proc @%d", ms);
	}
	int inter;
   
	graphics_context_set_fill_color(ctx, colorbg);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
	graphics_context_set_fill_color(ctx, colorticks);
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);
	graphics_context_set_stroke_color(ctx, colorticks);

	date_update_proc2();

	// Draw minutes traits
	inter=exterior - traitsize;
	for (int i = 0; i < 60; ++i) {
		GPoint P1 = {
			.x = sin_lookup(TRIG_MAX_ANGLE*i/60) * inter / TRIG_MAX_RATIO + center.x,
			.y = -cos_lookup(TRIG_MAX_ANGLE*i/60) * inter / TRIG_MAX_RATIO + center.y
			};
		GPoint P2 = {
			.x = sin_lookup(TRIG_MAX_ANGLE*i/60) * exterior / TRIG_MAX_RATIO + center.x,
			.y = -cos_lookup(TRIG_MAX_ANGLE*i/60) * exterior / TRIG_MAX_RATIO + center.y
			};
		graphics_draw_line(ctx, P1, P2);
	}

	//Draw points
	graphics_context_set_fill_color(ctx, colorpoints);
	graphics_context_set_stroke_color(ctx, colorpoints);
	inter=exterior - 6 - pointsize - traitsize;
	for (int i = 5; i < 60; i+=5) {
		GPoint P1 = {
			.x = sin_lookup(TRIG_MAX_ANGLE*i/60) * inter / TRIG_MAX_RATIO + center.x,
			.y = -cos_lookup(TRIG_MAX_ANGLE*i/60) * inter / TRIG_MAX_RATIO + center.y
			};
		graphics_fill_circle(ctx, P1, pointsize);
	}

	// Draw triangle
	//graphics_context_set_stroke_color(ctx, colorpoints);
	if (hi == 0) {
		Layer *text_layerhi = text_layer_get_layer(s_numhi_label);
		layer_set_hidden(text_layerhi,true);
		Layer *text_layer = text_layer_get_layer(s_num_label);
		layer_set_hidden(text_layer,false);
		GPoint P3 = {center.x,7};
		gpath_move_to(s_triangle, P3);
		gpath_draw_filled(ctx, s_triangle);
	} else {
		Layer *text_layerhi = text_layer_get_layer(s_numhi_label);
		layer_set_hidden(text_layerhi,false);
		Layer *text_layer = text_layer_get_layer(s_num_label);
		layer_set_hidden(text_layer,true);
	}

	//Draw battery traits
	for (int i = 0; i <11 ; ++i) {
		GPoint P1 = {
			.x = sin_lookup(TRIG_MAX_ANGLE*(i-5)/20) * 20 / TRIG_MAX_RATIO + center.x,
			.y = -cos_lookup(TRIG_MAX_ANGLE*(i-5)/20) * 20 / TRIG_MAX_RATIO + center.y+50
			};
		GPoint P2 = {
			.x = sin_lookup(TRIG_MAX_ANGLE*(i-5)/20) * 24 / TRIG_MAX_RATIO + center.x,
			.y = -cos_lookup(TRIG_MAX_ANGLE*(i-5)/20) * 24 / TRIG_MAX_RATIO + center.y+50
			};
		graphics_context_set_stroke_color(ctx, colorticks);
		graphics_draw_line(ctx, P1, P2);    
	}

	// Draw battery hand
	//if (debug==2) {APP_LOG(APP_LOG_LEVEL_INFO, "Battery:%d", s_battery_level);}
	battery_callback(battery_state_service_peek());
	GPoint P4 = {
		.x = sin_lookup(TRIG_MAX_ANGLE*(s_battery_level / 10 -5)/20) * -5 / TRIG_MAX_RATIO + center.x,
		.y = -cos_lookup(TRIG_MAX_ANGLE*(s_battery_level / 10 -5)/20) * -5 / TRIG_MAX_RATIO + center.y+50
		};
	GPoint P5 = {
		.x = sin_lookup(TRIG_MAX_ANGLE*(s_battery_level / 10 -5)/20) * 24 / TRIG_MAX_RATIO + center.x,
		.y = -cos_lookup(TRIG_MAX_ANGLE*(s_battery_level / 10 -5)/20) * 24 / TRIG_MAX_RATIO + center.y+50
		};
	//graphics_context_set_stroke_color(ctx, colorticks);
	graphics_draw_line(ctx, P4, P5);    
	if (debug==2) {
		uint16_t ms = time_ms(NULL,NULL);		
		APP_LOG(APP_LOG_LEVEL_INFO, "End bg_update_proc @%d", ms);
	}
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
	if (debug==2) {
		uint16_t ms = time_ms(NULL,NULL);		
		APP_LOG(APP_LOG_LEVEL_INFO, "Begin hands_update_proc @%d", ms);
	}

	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	if (printscreen == 1) {
		t->tm_hour = 10;
		t->tm_min = 10;
	}
	if (printscreen == 2) {
		t->tm_hour = 10;
		t->tm_min = 15;
	}
	//if (t->tm_hour > 7) {btvibe = 1;} else { btvibe = 0; }

	// second hand
	if (count>0) {
		if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "Seconds before hiding second hand %d", count);}
		secondhand=1;
		count--;
		int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
		GPoint second_hand = {
		  .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
		  .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
		};
		graphics_context_set_stroke_color(ctx, colorticks);
		graphics_draw_line(ctx, second_hand, center);
	} else {
		if (secondhand>0) {
			secondhand = 0;
			tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick);
			}
	}

	// minute/hour hand
	graphics_context_set_stroke_color(ctx, colorbg);
	gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
	minuteangle = (int) t->tm_min;
	graphics_context_set_fill_color(ctx, colorminute);
	gpath_draw_filled(ctx, s_minute_arrow);
	//  gpath_draw_outline(ctx, s_minute_arrow1);

	//if (colorminute!=colorhour) and (debug==2) {APP_LOG(APP_LOG_LEVEL_INFO, "colorminute!=colorhour");}

	gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
	hourangle = (int) (60 * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
	graphics_context_set_fill_color(ctx, colorhour);
	gpath_draw_filled(ctx, s_hour_arrow);
	//  gpath_draw_outline(ctx, s_hour_arrow1);

	// dot in the middle
	graphics_context_set_fill_color(ctx, colorminute);
	graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);

	if (debug==2) {
		uint16_t ms = time_ms(NULL,NULL);		
		APP_LOG(APP_LOG_LEVEL_INFO, "End hands_update_proc @%d", ms);
	}
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
	//if (count > 5*countmax) {count=count+30*countmax;}
	if (count > countmax) {count=count+10*countmax;}
	if (count > 0) {count=count+3*countmax;}
	if (count == 0) {count=countmax;}
	if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "tap_handler");}

	tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
	
	if (colorscheme<0) {colorscheme = colorscheme+3;}
	if (handscheme<0) {handscheme = handscheme+3;}
	switch (axis) {
	case ACCEL_AXIS_X:
		APP_LOG(APP_LOG_LEVEL_INFO, "X axis");
		colorscheme++;
		variables(colorscheme,handscheme);
		layer_mark_dirty(window_get_root_layer(window));
		//date_update_proc2();
		if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "colorscheme=%d",colorscheme);}
		if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "handscheme=%d",handscheme);}
		break;
	case ACCEL_AXIS_Y:
		break;
	case ACCEL_AXIS_Z:
		break;
	}
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	s_simple_bg_layer = layer_create(bounds);
	layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
	layer_add_child(window_layer, s_simple_bg_layer);

	// Create the Bluetooth icon GBitmap
	s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON_W);

	// Create the BitmapLayer to display the GBitmap
	s_bt_icon_layer = bitmap_layer_create(GRect(59, 40, 30, 30));
	bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));

	s_date_layer = layer_create(bounds);
	//layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);

	s_day_label = text_layer_create(GRect(0, 66, 42, 38));
	text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

	s_num_label = text_layer_create(GRect(103, 64, 42, 38));
	text_layer_set_text_alignment(s_num_label, GTextAlignmentRight);
	text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
	layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));

	s_numhi_label = text_layer_create(GRect(51, 4, 42, 38));
	text_layer_set_text_alignment(s_numhi_label, GTextAlignmentCenter);
	text_layer_set_font(s_numhi_label, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
	layer_add_child(s_date_layer, text_layer_get_layer(s_numhi_label));

	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);
}

static void window_unload(Window *window) {
	layer_destroy(s_simple_bg_layer);
	layer_destroy(s_date_layer);

	text_layer_destroy(s_day_label);
	text_layer_destroy(s_num_label);
	text_layer_destroy(s_numhi_label);

	layer_destroy(s_hands_layer);

	gbitmap_destroy(s_bt_icon_bitmap);
	bitmap_layer_destroy(s_bt_icon_layer);
}

static void init() {
	if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "INIT - BEGIN");}
  
	// Register for Bluetooth connection updates
	#ifdef PBL_SDK_2
	bluetooth_connection_service_subscribe(bluetooth_callback);
	#elif PBL_SDK_3
	connection_service_subscribe((ConnectionHandlers) {
		.pebble_app_connection_handler = bluetooth_callback
	});
	#endif

	if (debug==2) {APP_LOG(APP_LOG_LEVEL_INFO, "INIT - BT1 OK");}

	variables(colorscheme,handscheme);

	// Register for battery level updates
	battery_state_service_subscribe(battery_callback);
	accel_tap_service_subscribe(tap_handler);

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true);
	if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "INIT - WINDOWS OK");}

	s_day_buffer[0] = '\0';
	s_num_buffer[0] = '\0';
	s_numhi_buffer[0] = '\0';

	GPathInfo HOUR_HAND_POINTS = {
	5, (GPoint []){
		  {-hourhandbase, hourhandbase},
		  {hourhandbase, hourhandbase},
		  {hourhandwidth, (int) -hourlenght * 0.7},
		  {0, -hourlenght},
		  {-hourhandwidth, (int) -hourlenght * 0.7}
		}
	};

	GPathInfo MINUTE_HAND_POINTS = {
	5, (GPoint []){
		  {-minutehandbase, minutehandbase},
		  {minutehandbase, minutehandbase},
		  {minutehandwidth, (int) -minutelenght * 0.7},
		  {0, -minutelenght},
		  {-minutehandwidth, (int) -minutelenght * 0.7}
		}
	};

	GPathInfo TRIANGLE = {
	3, (GPoint []){
		  {-hourhandwidth, 0},
		  {hourhandwidth, 0},
		  {0, 4+2*abs(hourhandwidth)}
		}
	};
	s_triangle = gpath_create(&TRIANGLE);   

	// init hand paths
	s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
	s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
	if (debug==2) {APP_LOG(APP_LOG_LEVEL_INFO, "INIT - HANDS OK");}

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	GPoint center = grect_center_point(&bounds);
	gpath_move_to(s_minute_arrow, center);
	gpath_move_to(s_hour_arrow, center);
	if (debug==2) {APP_LOG(APP_LOG_LEVEL_INFO, "INIT - END");}
	tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick);
	//tick_timer_subscribe();
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	
	if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick)");}
	if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "MINUTE_UNIT:%d", MINUTE_UNIT);}
	if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "SECOND_UNIT:%d", SECOND_UNIT);}

	// Show the correct state of the BT connection from the start
	#ifdef PBL_SDK_2
	bluetooth_callback(bluetooth_connection_service_peek());
	#elif PBL_SDK_3
	bluetooth_callback(connection_service_peek_pebble_app_connection());
	#endif

	if (debug==2) {APP_LOG(APP_LOG_LEVEL_INFO, "INIT - BT2 OK");}
}

static void deinit() {
	gpath_destroy(s_minute_arrow);
	gpath_destroy(s_hour_arrow);
	gpath_destroy(s_triangle);

	tick_timer_service_unsubscribe();
	accel_tap_service_unsubscribe();

	window_destroy(window);

	if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "DEINIT - APP CLOSURE");}
}

int main() {
	init();
	app_event_loop();
	deinit();
}
