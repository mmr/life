/*
 *
 * $Id: life.c,v 1.9 2006/01/12 22:32:39 mmr Exp $
 * <mmr@b1n.org> in 2004-03-09 16:33:57 (BRST)
 *
 */

/* Libs */
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
  #include <time.h>
#else
  #include <sys/time.h>
#endif

#ifdef __APPLE__
  #include  <OpenGL/gl.h>
  #include  <OpenGL/glu.h>
  #include  <GLUT/glut.h>
#else
  #include  <GL/gl.h>
  #include  <GL/glu.h>
  #include  <GL/glut.h>
#endif

/* Defines */
#define b1n_VSIZE   500
#define b1n_HSIZE   500
#define b1n_TIME    333
#define b1n_GRID_X  10
#define b1n_GRID_Y  10
#define b1n_GRIDSPACING_X (GLfloat)2/b1n_GRID_X
#define b1n_GRIDSPACING_Y (GLfloat)2/b1n_GRID_Y

/* Macros */
#define b1n_RANDCOLOR\
  glColor3b(\
    255*((GLfloat)rand()/RAND_MAX),\
    255*((GLfloat)rand()/RAND_MAX),\
    255*((GLfloat)rand()/RAND_MAX));

#define b1n_DRAWLINE(x0,y0,x1,y1)\
  glVertex2f(x0,y0);\
  glVertex2f(x1,y1);

/* Types */
typedef struct {
  float x, y;
  char  r, g, b;
  unsigned int alive:1;
} b1n_cell;

/* Prototypes */
void b1n_initLife(void);
void b1n_liveAndLetDie(void);
void b1n_showCell(unsigned int, unsigned int);
void b1n_hideCell(unsigned int, unsigned int);
void b1n_drawGrid(void);

  /* Call Back */
void b1n_reshape(int, int);
void b1n_timer(int);
void b1n_mouse(int button, int state, int x, int y);
void b1n_zoom(int x, int y);
void b1n_motion(int x, int y);
void b1n_keyboard(unsigned char, int, int);
static void b1n_specialkey(int key, int x, int y);

/* Global Vars */
b1n_cell g_cells[b1n_GRID_X+1][b1n_GRID_Y+1];

int g_beginx = 0, g_beginy = 0;
GLfloat g_anglexmodel = 0, g_angleymodel = 0;
int g_zoombegx = 0, g_zoombegy = 0;
GLfloat g_zoomx = 0, g_zoomy = 0;

  /* Flags */
int g_moving = GL_FALSE;
int g_color = GL_FALSE;
int g_grid  = GL_FALSE;
int g_reset = GL_FALSE;
int g_rotate = GL_FALSE;
int g_zoom  = GL_FALSE;

/* Main */
int
main(int argc, char **argv)
{
  /* Create Window and Intialize Glut */
  glutInitWindowSize(b1n_VSIZE, b1n_HSIZE);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutCreateWindow("mmr's Game of Life");

  /* Initialize Life */
  b1n_initLife();
  
  /* Configuring Backend */
  glutReshapeFunc(b1n_reshape);
  //glutDisplayFunc(b1n_liveAndLetDie);
  glutTimerFunc(b1n_TIME, b1n_timer, 1);
 // glutIdleFunc(b1n_liveAndLetDie);
 // glutMouseFunc(b1n_mouse);
 // glutMotionFunc(b1n_motion);
  glutSpecialFunc(b1n_specialkey);
  glutKeyboardFunc(b1n_keyboard);

  /* Loop */
  glutMainLoop();
  return 0;
}

/*
void 
b1n_reshape(GLint w, GLint h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(35.0, (GLfloat) w / (GLfloat) h, 1.0, 20.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -5.0);
}
*/

void
b1n_reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30, (GLfloat) w/(GLfloat) h, 1.0, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);
}

void
b1n_keyboard(unsigned char key, int x, int y)
{
  switch(key){
  case 'g': /* Toggle Grid */
    g_grid = !g_grid;
    break;
  case 'r': /* Reset */
    g_reset = GL_TRUE;
    break;
  case 't': /* Toggle Rotation */
    g_rotate = !g_rotate;
    break;
  case 'T': /* Reset Rotation */
    g_anglexmodel = 0;
    g_angleymodel = 0;
    break;
  case 'c': /* Toggle Randcolor */
    g_color = !g_color;
    break;
  case 'Q': /* Quit */
  case 'q':
  case 27:
    exit(1);
  }
  /* glutPostRedisplay(); */
}

void
b1n_initLife(void)
{
  int   x,y;
  float xf,yf;
  float xinc,yinc;

#ifdef _WIN32
  srand(time(NULL));
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);
#endif

  glClearColor(0.0, 0.0, 0.0, 0.0);

  if(g_grid){
    b1n_drawGrid();
  }

  xinc = (GLfloat)b1n_GRIDSPACING_X/2;
  yinc = (GLfloat)b1n_GRIDSPACING_Y/2;
  glPointSize(3.0);

  /* Initializing the 0 and grid+1 lines (that arent displayed but needed) */
  for(x=0,y=b1n_GRID_X; x<b1n_GRID_X; x++,y--){
    g_cells[x][0].alive = GL_FALSE;
    g_cells[y][0].alive = GL_FALSE;
  }
  for(x=0,y=b1n_GRID_X; x<b1n_GRID_X; x++,y--){
    g_cells[0][x].alive = GL_FALSE;
    g_cells[0][y].alive = GL_FALSE;
  }

  /* Initializing the displayed cells */
  for(x=1,xf=xinc-1.0; x<=b1n_GRID_X; x++,xf+=b1n_GRIDSPACING_X){
    for(y=1,yf=1.0-yinc; y<=b1n_GRID_Y; y++,yf-=b1n_GRIDSPACING_Y){
      g_cells[x][y].r = 255*((GLfloat)rand()/RAND_MAX);
      g_cells[x][y].g = 255*((GLfloat)rand()/RAND_MAX);
      g_cells[x][y].b = 255*((GLfloat)rand()/RAND_MAX);
      g_cells[x][y].x = xf;
      g_cells[x][y].y = yf;
      g_cells[x][y].alive = GL_FALSE;
      if(rand()%5==1){
        b1n_showCell(x,y);
      }
    }
  }
}

void
b1n_timer(int value)
{
  b1n_liveAndLetDie();
  glutPostRedisplay();
  glutTimerFunc(b1n_TIME, b1n_timer, 1);
}

void
b1n_liveAndLetDie(void)
{
  int c;
  unsigned int x,y;

  glClear(GL_COLOR_BUFFER_BIT);

  /* Reset */
  if(g_reset){
    g_reset = GL_FALSE;
    b1n_initLife();
  }

  /* Rotation */
  if(g_rotate){
    glRotatef(g_anglexmodel, 0.0, 1.0, 0.0);
    glRotatef(g_angleymodel, 1.0, 0.0, 0.0);
  }
  else {
    glRotatef(0, 0.0, 1.0, 0.0);
    glRotatef(0, 1.0, 0.0, 0.0);
  }

  /* Grid */
  if(g_grid){
    b1n_drawGrid();
  }

  /* Cells */
  for(x=1;x<=b1n_GRID_X;x++){
    for(y=1;y<=b1n_GRID_Y;y++){
      c = g_cells[x-1][y+1].alive +
          g_cells[ x ][y+1].alive +
          g_cells[x+1][y+1].alive +
          g_cells[x-1][ y ].alive +
          g_cells[x+1][ y ].alive +
          g_cells[x-1][y-1].alive +
          g_cells[ x ][y-1].alive +
          g_cells[x+1][y-1].alive;

      switch(c){
      case 2:
        if(rand()%7==0){
          b1n_hideCell(x,y);
        }
        break;
      case 3:
        b1n_showCell(x,y);
        break;
      default:
        b1n_hideCell(x,y);
      }
    }
  }

  /* Swapping Buffers */
  glutSwapBuffers();
}

void
b1n_showCell(unsigned int x, unsigned int y)
{
  g_cells[x][y].alive = GL_TRUE;

  glBegin(GL_POINTS);
    if(g_color){
      glColor3b(g_cells[x][y].r, g_cells[x][y].g, g_cells[x][y].b);
    }
    else{
      glColor3f(0.0, 1.0, 0.0);
    }
    glVertex2f(g_cells[x][y].x, g_cells[x][y].y);
  glEnd();
}

void
b1n_hideCell(unsigned int x, unsigned int y)
{
  g_cells[x][y].alive = GL_FALSE;

  glBegin(GL_POINTS);
    glColor3f(0.0, 0.0, 0.0);
    glVertex2f(g_cells[x][y].x, g_cells[x][y].y);
  glEnd();
}

void
b1n_drawGrid(void)
{
  float x, y;

  glColor3f(0.2,0.2,0.2);
  glBegin(GL_LINES);
    for(x=-1.0; x<1.0; x+=b1n_GRIDSPACING_X){
      b1n_DRAWLINE(x,-1,x,1);
    }
    for(y=-1.0; y<1.0; y+=b1n_GRIDSPACING_Y){
      b1n_DRAWLINE(-1,y,1,y);
    }
  glEnd();
}

void
b1n_mouse(int button, int state, int x, int y)
{
  if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
    g_moving = GL_TRUE;
    g_beginx = x;
  }
  if(button == GLUT_LEFT_BUTTON && state == GLUT_UP){
    g_moving = GL_FALSE;
  }
}

void
b1n_motion(int x, int y)
{
  if(g_moving){
    g_anglexmodel += (x - g_beginx);
    g_angleymodel += (y - g_beginy);
    g_beginx = x;
    g_beginy = y;
    /* glutPostRedisplay(); */
  }
}

void
b1n_zoom(int x, int y)
{
  if(g_zoom){
    g_zoomx += (x - g_zoombegx);
    g_zoomy += (y - g_zoombegy);
    g_zoombegx = x;
    g_zoombegy = y;
  }
}

static void
b1n_specialkey(int key, int x, int y)
{
  int oldmoving = g_moving;

  g_moving = GL_TRUE;
  switch(key){
  case GLUT_KEY_LEFT:
    g_rotate = GL_TRUE;
    g_beginx = g_beginy = 0;
    b1n_motion(-1,0);
    break;
  case GLUT_KEY_RIGHT:
    g_rotate = GL_TRUE;
    g_beginx = g_beginy = 0;
    b1n_motion(1,0);
    break;
  case GLUT_KEY_UP:
    g_rotate = GL_TRUE;
    g_beginx = g_beginy = 0;
    b1n_motion(0,-1);
    break;
  case GLUT_KEY_DOWN:
    g_rotate = GL_TRUE;
    g_beginx = g_beginy = 0;
    b1n_motion(0,1);
    break;
  case 'z': /* Zoom In */
    g_zoom = GL_TRUE;
    g_zoombegx = g_zoombegy = 0;
    b1n_zoom(1,0);
    break;
  case 'Z': /* Zoom Out */
    g_zoom = GL_TRUE;
    g_zoombegx = g_zoombegy = 0;
    b1n_zoom(-1,0);
    break;
  }
  g_moving=oldmoving;
}
