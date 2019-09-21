
#include "fftrefraction.h"
#include "matrix.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define TREE_COUNT 60
#define TREE_DENSITY 1
#define WATER_LEVEL 10

struct the_forest
{
	int	tree_position[3];
	double angle;	//this is in 5 degree increments
	int image;// this is 0 thru 35 (36 images)
} trees[TREE_COUNT];

double my_x=127.,my_y=130.,my_z=-135.;

char *tree_filenames[] = {"g000.tga","g010.tga","g020.tga","g030.tga","g040.tga","g050.tga","g060.tga","g070.tga","g080.tga",
	"g090.tga","g100.tga","g110.tga","g120.tga","g130.tga","g140.tga","g150.tga","g160.tga","g170.tga","g180.tga","g190.tga","g200.tga",
	"g210.tga","g220.tga","g230.tga","g240.tga","g250.tga","g260.tga","g270.tga","g280.tga","g290.tga","g300.tga",
	"g310.tga","g320.tga","g330.tga","g340.tga","g350.tga"};


//
// read the TGA tree images
//
void Alaska::LoadTreeTextures()
{
	glGenTextures(36,texnames);

	//master loop to bring the textures in
	for (int xx=0; xx<36; xx++)	//where xx is the number associated with each texture
	{
		strcpy(file_name, "tgatrees\\");
		strcat(file_name, tree_filenames[xx]);

		LoadTGAFile( file_name, &m_ReflectImage[xx]);

		// create the alpha channel by adding a byte to the end of each width RGB, giving RGBA
		alpha_point[xx] = (GLubyte *) malloc(pixel_width*4*pixel_height);
		if (alpha_point[xx] == NULL)
			printf("failure to malloc");

		//this loop transfers the rgb to the rgba pointer so alpha blending can be used
		for (int i=0; i<pixel_width*pixel_height; i++)
		{
			*(alpha_point[xx]+i*4)=(GLubyte)*(m_ReflectImage[xx].data+i*3);
			*(alpha_point[xx]+i*4+1)=(GLubyte)*(m_ReflectImage[xx].data+i*3+1);
			*(alpha_point[xx]+i*4+2)=(GLubyte)*(m_ReflectImage[xx].data+i*3+2);
		//	*(alpha_point[xx]+i*4+3)=(GLubyte)0;
	
			if (*(alpha_point[xx]+i*4)<=(GLubyte)alpha_color_array[0] &&
				*(alpha_point[xx]+i*4+1)<=(GLubyte)alpha_color_array[1] &&
				*(alpha_point[xx]+i*4+2)<=(GLubyte)alpha_color_array[2])
			{
				*(alpha_point[xx]+i*4+3)=(GLubyte)0;
			}
			else
			{
				*(alpha_point[xx]+i*4+3)=(GLubyte)255;
			}
		}

		glBindTexture(GL_TEXTURE_2D,texnames[xx]);

		//following stuff added to get my surface to have texture
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixel_width,  //pixel_width=width in pixels
		pixel_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		alpha_point[xx]);
		glEnable(GL_TEXTURE_2D);
	} // this is the end of the xx loop
}


void Alaska::make_trees()
{
	int xtree, ytree;
	srand(time(0));
	for (int i=0;i<TREE_COUNT;i++)
	{
		while(1)
		{
try_again:
			xtree=MINX+rand()%MAXX;//gives array indices
			ytree=MINY+rand()%MAXY;
			if (land(xtree, ytree, 1) <= WATER_LEVEL+.4)
				goto try_again;
			for (int j=0;j<i;j++)
			{
				if ((abs(trees[j].tree_position[0]-xtree)<TREE_DENSITY) &&
					(abs(trees[j].tree_position[1]-ytree)<TREE_DENSITY))
					goto try_again;
			}
			trees[i].tree_position[0]=land(xtree, ytree, 0);
			trees[i].tree_position[1]=land(xtree, ytree, 1);
			trees[i].tree_position[2]=land(xtree, ytree, 2);
			trees[i].image=rand()%36;
			break;
		}
	}
}

void Alaska::forest()
{
	int i;
	double norvec[3];

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glColor4f(1,1,1,1);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	for (i=0;i<TREE_COUNT;i++)
	{
		glPushMatrix();

		//these are wrong. I need actual tree space positions to do the dot product
		// I have to find a way to calculate them
		//I should fix all of the tree routine. It's too cumbersome and mysterious.
		double deltax=(double)trees[i].tree_position[0]-(my_x);
		double deltay=(double)trees[i].tree_position[1]-(my_y);
		double deltaz=(double)trees[i].tree_position[2]-(my_z);
		norvec[0]=deltax;norvec[1]=deltay;norvec[2]=deltaz;
		normalize(norvec);
		double rotangle=acos(norvec[2])/TO_RADS;

	//	double range=deltax*deltax+deltay*deltay;
	//	double rotangle=(deltay<0.? 360.+atan2(deltay,deltax)/TO_RADS :atan2(deltay,deltax)/TO_RADS);
		glTranslatef(trees[i].tree_position[0],trees[i].tree_position[1],trees[i].tree_position[2]);
	//	glRotatef(90.0+rotangle,0,1,0);//this does maintain the correct orientation
		glRotatef(rotangle,0,0,1);

		trees[i].angle=rotangle;
		glBindTexture(GL_TEXTURE_2D,texname[trees[i].image]);//use data from struct

		glDisable(GL_LIGHTING);//turn on
		glEnable(GL_ALPHA_TEST);//turn on
		glAlphaFunc(GL_LESS, .8);

	//	z=gHeightMap[trees[i].tree_position[0]+trees[i].tree_position[1]*MAP_SIZE+trees[i].tree_position[2]];
	//	z=trees[i].tree_position[2];

		glBegin(GL_QUADS);
		glTexCoord2f(0,1);
	//	glVertex3f(-3,0,z-7);//these lines are the tree width/height/depth controls
		glVertex3f(-30,-70,0);

		glTexCoord2f(0,0);
	//	glVertex3f(-3,0,z+15);
		glVertex3f(-30,+150,0);


		glTexCoord2f(1,0);
	//	glVertex3f(3,0,z+15);
		glVertex3f(30,+150,0);

		glTexCoord2f(1,1);
	//	glVertex3f(3,0,z-7);
		glVertex3f(30,-70,0);

		glEnd();

		glPopMatrix();
	}

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
}

