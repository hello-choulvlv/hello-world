
//   fftrefraction.cpp visualize paul bourke's fft. This program takes Jensen into account
//	 joe dart ffjjd@uaf.edu	 It also uses Chen water ideas and some Nutman cubemap code
//		This program is first version of cg refraction.		
//		This program runs correctly with "choppy" waves.
//		june 22, 2003

#include "fftrefraction.h"
#include "matrix.h"

#include <GL\glaux.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>	// needed to use _timeb
#include <sys/timeb.h>	// needed to use _timeb
#include <cg/cg.h>
#include <cg/cggl.h>
#include "loadTGA.h"
extern "C"
{
#include "jpeglib.h"
}

#include <iostream>
using namespace std;

