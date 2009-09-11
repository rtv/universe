/****
   universe.c
   version 2
   Richard Vaughan  
****/

#include <assert.h>
#include <unistd.h>

#include "universe.h"
using namespace Uni;

const char* PROGNAME = "universe";

#if GRAPHICS
#include <GLUT/glut.h> // OS X users need <glut/glut.h> instead
int Robot::winsize( 500 );
int Robot::displaylist(0);
bool Robot::show_data( true );
#endif

// initialize static members
double Robot::worldsize(0.8);
double Robot::range( 0.1 );
double Robot::fov(  M_PI/10.0 );
//double Robot::fov( 2.0 * M_PI );
std::vector<Robot*> Robot::population;
unsigned int Robot::pixel_count( 32 );
unsigned int Robot::sleep_msec( 10 );
uint64_t Robot::updates(0);
uint64_t Robot::updates_max( 0.0 ); 
bool Robot::paused( false );

#if GRAPHICS
// GLUT callback functions ---------------------------------------------------

// update the world - this is called whenever GLUT runs out of events
// to process
static void idle_func( void )
{
  Robot::UpdateAll();
}

static void timer_func( int dummy )
{
  glutPostRedisplay(); // force redraw
}

// draw the world - this is called whenever the window needs redrawn
static void display_func( void ) 
{  
  glClear( GL_COLOR_BUFFER_BIT );  
  Robot::DrawAll();
  glutSwapBuffers();
	
  // run this function again in about 50 msec
  glutTimerFunc( 20, timer_func, 0 );
}

static void mouse_func(int button, int state, int x, int y) 
{  
  if( (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN ) )
	 {
		Robot::paused = !Robot::paused;
	 }
}

// render all robots in OpenGL
void Robot::DrawAll()
{
	FOR_EACH( r, population )
		(*r)->Draw();
}

#endif // GRAPHICS

Robot::Robot( const Pose& pose, 
				  const Color& color )
  : pose(pose),
	 speed(),
	 color(color),
	 pixels()
{
  // add myself to the static vector of all robots
  population.push_back( this );
  pixels.resize( pixel_count );
}

void Robot::Init( int argc, char** argv )
{
  // parse arguments to configure Robot static members
  // TODO
 
  // seed the random number generator with the current time
  srand48(time(NULL));

#if GRAPHICS
  // initialize opengl graphics
  glutInit( &argc, argv );
  glutInitWindowSize( winsize, winsize );
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );
  glutCreateWindow( PROGNAME );
  glClearColor( 0.8,0.8,1.0,1.0 );
  glutDisplayFunc( display_func );
  glutTimerFunc( 50, timer_func, 0 );
  glutMouseFunc( mouse_func );
  glutIdleFunc( idle_func );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_BLEND );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0,1,0,1 );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glScalef( 1.0/Robot::worldsize, 1.0/Robot::worldsize, 1 ); 
  
  // define a display list for a robot body
  double h = 0.01;
  double w = 0.01;

	glPointSize( 4.0 );

  displaylist = glGenLists(1);
  glNewList( displaylist, GL_COMPILE );

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  glBegin( GL_POLYGON );
  glVertex2f( h/2.0, 0 );
  glColor3f( 0,0,0 ); // black
  glVertex2f( -h/2.0,  w/2.0 );
  glVertex2f( -h/2.0, -w/2.0 );
  glEnd();

  glEndList();
#endif // GRAPHICS
}

void Robot::UpdatePixels()
{
  double radians_per_pixel = fov / (double)pixel_count;
  
  // initialize pixels vector  
  FOR_EACH( it, pixels )
	 {
		it->range = Robot::range; // maximum range
      it->robot = NULL; // nothing detected
    }
  
  // check every robot in the world to see if it is detected
  FOR_EACH( it, population )
    {
      Robot* other = *it;
      
      // discard if it's the same robot
      if( other == this )
				continue;
			
      // discard if it's out of range. We put off computing the
      // hypotenuse as long as we can, as it's relatively expensive.
			
      double dx = other->pose.x - pose.x;
			if( dx > Robot::range )
				continue; // out of range
			
      double dy = other->pose.y - pose.y;
			if( dy > Robot::range )
				continue; // out of range
			
      double range = hypot( dx, dy );
      if( range > Robot::range ) 
				continue; 
			
      // discard if it's out of field of view 
      double absolute_heading = atan2( dy, dx );
      double relative_heading = AngleNormalize((absolute_heading - pose.a) );
      if( fabs(relative_heading) > fov/2.0   ) 
				continue; 
			
      // find which pixel it falls in 
      int pixel = floor( relative_heading / radians_per_pixel );
      pixel += pixel_count / 2;
      pixel %= pixel_count;

      assert( pixel >= 0 );
      assert( pixel < (int)pixel_count );

      // discard if we've seen something closer in this pixel already.
      if( pixels[pixel].range < range) 
		  continue;
		
      // if we made it here, we see this other robot in this pixel.
      pixels[pixel].range = range;
      pixels[pixel].robot = other;
    }	
}

void Robot::UpdatePose()
{
  // move according to the current speed 
  double dx = speed.v * cos(pose.a);
  double dy = speed.v * sin(pose.a);; 
  double da = speed.w;
  
  pose.x = DistanceNormalize( pose.x + dx );
  pose.y = DistanceNormalize( pose.y + dy );
  pose.a = AngleNormalize( pose.a + da );
}

void Robot::UpdateAll()
{
  // if we've done enough updates, exit the program
  if( updates > updates_max )
	 exit(1);
  
  if( ! Robot::paused )
		{
			FOR_EACH( r, population )
				(*r)->UpdatePose();

			FOR_EACH( r, population )
				(*r)->UpdatePixels();

			FOR_EACH( r, population )
				(*r)->Controller();
		}
  
  // possibly snooze to save CPU and slow things down
  if( sleep_msec > 0 )
	 usleep( sleep_msec * 1e3 );
}

// draw a robot
void Robot::Draw()
{
#if GRAPHICS
  glPushMatrix();
  glTranslatef( pose.x, pose.y, 0 );
  glRotatef( rtod(pose.a), 0,0,1 );
  
	glColor3f( color.r, color.g, color.b ); 

	// draw the pre-compiled triangle for a body
  glCallList(displaylist);
  
  if( Robot::show_data )
	 {
		// render the sensors
		double rads_per_pixel = fov / (double)pixel_count;
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		
		for( unsigned int p=0; p<pixel_count; p++ )
		  {
				double angle = -fov/2.0 + (p+0.5) * rads_per_pixel;
				double dx1 = pixels[p].range * cos(angle+rads_per_pixel/2.0);
				double dy1 = pixels[p].range * sin(angle+rads_per_pixel/2.0);
				double dx2 = pixels[p].range * cos(angle-rads_per_pixel/2.0);
				double dy2 = pixels[p].range * sin(angle-rads_per_pixel/2.0);
				
				glColor4f( 1,0,0, pixels[p].robot ? 0.2 : 0.05 );
				
				glBegin( GL_POLYGON );
				glVertex2f( 0,0 );
				glVertex2f( dx1, dy1 );
				glVertex2f( dx2, dy2 );
				glEnd();                  
		  }	  
	 }

  glPopMatrix();
#endif // GRAPHICS
}


void Robot::Run()
{
#if GRAPHICS
  glutMainLoop();
#else
  while( 1 )
    UpdateAll();
#endif
}

/** Normalize a length to within 0 to worldsize. */
double Robot::DistanceNormalize( double d )
{
	while( d < 0 ) d += worldsize;
	while( d > worldsize ) d -= worldsize;
	return d; 
} 

/** Normalize an angle to within +/_ M_PI. */
double Robot::AngleNormalize( double a )
{
	while( a < -M_PI ) a += 2.0*M_PI;
	while( a >  M_PI ) a -= 2.0*M_PI;	 
	return a;
}	 
