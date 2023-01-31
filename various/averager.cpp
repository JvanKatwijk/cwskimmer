
#include	"averager.h"
#include	<cstring>

	average::average (int16_t s) {
int16_t	i;

	size	= s;
	vec	= new float [s];
	filp	= 0;

	memset (vec, 0, s * sizeof (float));
}

	average::~average () {
	delete[]	vec;
}

float	average::filter (float e) {
int16_t	i;
float	out	= 0.0;

	vec [filp] = e;
	filp = (filp + 1) % size;

	for (i = 0; i < size; i ++)
	   out += vec [i];

	return out / size;
}

void	average::clear (float c ) {
int16_t	i;

	for (i = 0; i < size; i ++)
	   vec [i] = c;
	filp = 0;
}

