#include "colors.h"

/*
	generates and returns a red channel color value;
	this must be an integer between 0 and 255 inclusive
*/
int generate_red(float ratio) {
	if (ratio == 1)
		return 20;
	return 0;
}

/*
	generates and returns a green channel color value;
	this must be an integer between 0 and 255 inclusive
*/
int generate_green(float ratio) {
	if(ratio == 1)
		return 0;
	else
		return (unsigned char)(255*ratio);
}

/*
	generates and returns a blue channel color value;
	this must be an integer between 0 and 255 inclusive
*/
int generate_blue(float ratio) {
	if(ratio == 1)
		return 0;
	else
		return (unsigned char)(255 - 255*ratio)/5;
}