#include <math.h>

#include "3dfsb.h"
#include "config.h"
#include "input.h"

/*
 * Input handling code
 */
void MouseMove(int x, int y)
{
	if (!TDFSB_ANIM_STATE) {
		centX = centX - (GLdouble) (headspeed * (floor(((double)SWX) / 2) - (double)x));
		centY = centY - (GLdouble) (headspeed * (floor(((double)SWY) / 2) - (double)y));
		if (centY >= SWY / 2)
			centY = SWY / 2;
		if (centY <= ((-SWY) / 2))
			centY = ((-SWY) / 2);
		tposz = ((GLdouble) sin((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposx = ((GLdouble) cos((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposy = -((GLdouble) sin((((double)centY) / (double)(SWY / PI))));
	}
}


