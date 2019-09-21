
#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "paulslib.h"
#include "LoadTGA.h"

struct SVertex3
{
	float x, y, z;
};

struct Vector3
{
	float x, y, z;
};

#define NX 64
#define NY 64
#define NXTIMESNY NX*NY

#define INV_SQRT_TWO (1.0f)/sqrt(2.0f)
#define MAX_WORLD_X 128
#define MAX_WORLD_Y 128
#define BIG_NX (NX+1)
#define BIG_NY (NY+1)
#define STEPX MAX_WORLD_X/NX

#define MINX 10
#define MAXX 100
#define MINY 10
#define MAXY 100

#define GRAV_CONSTANT	9.81f //gravitational constant metric
//#define GRAV_CONSTANT	22.81f //gravitational constant metric


/**
 * This class contains all the methods and data for the application.
 */
class Alaska
{
public:
	Alaska();
	~Alaska();

	void	CreateDynamicCubeMap(void);
	double	get_current_seconds();
	void	text(int x, int y, char* s);
	void	frame_counter();
	void	Cg_init();
	void	read_land_in();
	GLubyte *read_JPEG_file(char * filename);
	GLuint	Loadjpg(char * filename);
	GLuint	LoadDecalClampjpg(GLubyte * bptr,char * filename);
	GLuint	LoadRepeatjpg(GLubyte * bptr,char * filename);
	GLuint	LoadTexture2D(const char *const filename);	//reads BMP files
	GLuint	CreateCubemap(void);
	GLuint	CreateCubemapa(void);
	GLuint	CreateBottomMap(void);
	void	calculate_ho();
	GLuint	init_normals_texturemap();
	void	calculate_normal_texture_map();
	void	__MakeNormalizationCubeMap(const int size, const int level);
	GLuint	MakeNormalizationCubeMap(void);

	void set_cube_cg();
	void unset_cube_cg();
	void set_cg_water();
	void unset_cg_water();
	void draw_the_land();
	void RenderSkyBox(void);
	void show_normals();
	void RenderOcean();
	void display(void);
	void make_normals(COMPLEX c[NX][NY]);
	void myinit(void);
	void myReshape(int w, int h);
	void menu(int item);
	void special_menu(int item_too);
	void pre_choppy();
	void prep_loop();
	void idle(void);

	// forest
	void LoadTreeTextures();
	void make_trees(void);
	void forest(void);

protected:
	// forest
	GLuint texname[3];

	/////// surface ///////
	double *m_land;
	double *m_land_normals;
	int landusize,landvsize;
	// provide indexed access to the dynamically allocated land buffer
	double &land(int x, int y, int c)
	{
		return m_land[(x*landusize*3)+(y*3)+c];
	}
	double &land_normals(int x, int y, int c)
	{
		return m_land_normals[(x*landusize*3)+(y*3)+c];
	}

	float hold_modelview_matrix[16];
	int stripswitch;
	int shownormalsswitch;
	int panswitch;
	GLuint texnames[36];
	GLubyte *alpha_point[36];// hold tree images
	tTGAFile_s m_ReflectImage[36];

	//////////
	GLubyte alpha_color_array[3];
	int pixel_height,pixel_width;
	char file_name[25];
	int window_width;
	int window_height;
	GLuint tex[8];
	int row_width;		//returned from jpeg function. pixel width = row_width/3 due to RGB bytes/pixel
	int col_height;		// height of image returned from jpeg function
	double lambda;
	float fog_dense;
	double anglex;	//used to rock the boat
	double angley;	//used to rock the boat
	int dir;
	int frameswitch;

	float horiz,vert,rotx;
	int cgswitch;
	float wind;

	double factor;	//this determines speed of wave
	double start_time;

	GLubyte *txpnt;
};

// Helpers
double phillips(double a,double k[2],double wind[2]);
void my_normalize(Vector3 &vec);
void make_signedN(Vector3 &v, unsigned char *ip);

int	FFT(int,int,double *,double *);
int	FFT2D(COMPLEX [][NY], int,int,int);
int	DFT(int,int,double *,double *);
int	Powerof2(int,int *,int *);

void gauss(double mywork[2]);

void mouse(int b, int s, int x, int y);
void drag(int x, int y);
float	neg1Pow(int k);

