#include <pebble.h>

static void variables(int colorsh, int handsh) {
	if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "VARIABLES - BEGIN");}
	handsh = (handsh+3) % 3;
  	switch (handsh) {
		case 1: {
			  exterior = 90;
			  hourhandbase = 4;
			  hourhandwidth = 6;
			  hourlenght = 56;
			  minutehandbase = 3;
			  minutehandwidth = 5;
			  minutelenght = 85;
			  //maxminutelenght = 90;
			  second_hand_length = 75;
			  pointsize = 4;
			  traitsize = 8;
		  }
		  break;
		case 2: {
			  exterior = 90;
			  hourhandbase = 4;
			  hourhandwidth = -8;
			  hourlenght = 56;
			  minutehandbase = 3;
			  minutehandwidth = -6;
			  minutelenght = 90;
			  //maxminutelenght = 95;
			  second_hand_length = 75;
			  pointsize = 5;
			  traitsize = 10;
		  }
		  break;
		default: {
			  exterior = 90;
			  hourhandbase = 2;
			  hourhandwidth = 8;
			  hourlenght = 56;
			  minutehandbase = 2;
			  minutehandwidth = 6;
			  minutelenght = 85;
			  //maxminutelenght = 95;
			  second_hand_length = 75;
			  pointsize = 5;
			  traitsize = 10;
		  }
		  break;
	}

	colorsh = (colorsh+3) % 3;
  	switch (colorsh) {
		case 1: {
			// White
			#ifdef PBL_SDK_2
			colorbg = GColorWhite;
			colorticks = GColorBlack;
			colorpoints = GColorBlack;
			colorhour = GColorBlack;
			colorminute = GColorBlack;
			colortext = GColorBlack;
			colorbgtext = colorbg;
			#elif PBL_SDK_3 
			colorbg = GColorFromRGB(255,255,255);
			colorticks = GColorFromRGB(0,0,0);
			colorpoints = GColorFromRGB(0,0,128);
			colorhour = GColorFromRGB(255,0,0);
			colorminute = GColorFromRGB(0,0,128);
			colortext = GColorFromRGB(0,0,128);
			colorbgtext = GColorFromRGB(255,255,255);
			if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "colorbgtext white");}
			#endif
		  }
		  break;
		case 2: {
			// Colorfull
			#ifdef PBL_SDK_2
			colorbg = GColorBlack;
			colorticks = GColorWhite;
			colorpoints = GColorWhite;
			colorhour = GColorWhite;
			colorminute = GColorWhite;
			colortext = GColorBlack;
			colorbgtext = GColorWhite;
			#elif PBL_SDK_3 
			colorbg = GColorFromRGB(255,255,255);
			colorticks = GColorFromRGB(0,64,0);
			colorpoints = GColorFromRGB(0,64,0);
			colorhour = GColorFromRGB(255,0,0);
			colorminute = GColorFromRGB(0,0,128);
			colortext = GColorFromRGB(0,64,0);
			colorbgtext = GColorFromRGB(255,255,255);
			if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "colorbgtext white");}
			#endif
		  }
		  break;
		default: {
			// Black
			#ifdef PBL_SDK_2
			colorbg = GColorBlack;
			colorticks = GColorWhite;
			colorpoints = GColorWhite;
			colorhour = GColorWhite;
			colorminute = GColorWhite;
			colortext = GColorWhite;
			colorbgtext = colorbg;
			#elif PBL_SDK_3 
			colorbg = GColorBlack;
			colorticks = GColorWhite;
			colorpoints = GColorWhite;
			colorhour = GColorFromRGB(255,0,0);
			colorminute = GColorWhite;
			colortext = GColorWhite;
			colorbgtext = GColorBlack;
			if (debug>0) {APP_LOG(APP_LOG_LEVEL_INFO, "colorbgtext black");}
			#endif
		  }
		  break;
	}
}  



