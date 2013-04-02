/****
     universe.cc
     version 3
     Richard Vaughan  
****/

#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include "universe.h"
using namespace Uni;

const char* PROGNAME = "universe";

#if GRAPHICS
#include <GLUT/glut.h> // OS X users need <glut/glut.h> instead
#endif

namespace Uni {
  
  bool need_redraw( true );
  int32_t worldsize(1000);
  std::vector<Robot> population( 100 );
  uint64_t updates(0);
  uint64_t updates_max( 0.0 ); 
  bool paused( false );
  int winsize( 600 );
  int displaylist(0);
  bool show_data( true );
  unsigned int sleep_msec( 50 );
  
  double lastseconds; 

  // Robot static members
  unsigned int Robot::pixel_count(8);
  int32_t Robot::range( 100 );
  int32_t Robot::fov(  mdtomr(270000) ); // milliradians
}

char usage[] = "Universe understands these command line arguments:\n"
  "  -? : Prints this helpful message.\n"
  "  -c <int> : sets the number of pixels in the robots' sensor.\n"
  "  -d  Disables drawing the sensor field of view. Speeds things up a bit.\n"
  "  -f <float> : sets the sensor field of view angle in degrees.\n"
  "  -p <int> : set the size of the robot population.\n"
  "  -q : disables chatty status output (quiet mode).\n"
  "  -r <float> : sets the sensor field of view range.\n"
  "  -s <float> : sets the side length of the (square) world.\n"
  "  -u <int> : sets the number of updates to run before quitting.\n"
  "  -w <int> : sets the initial size of the window, in pixels.\n"
  "  -z <int> : sets the number of milliseconds to sleep between updates.\n";

#if GRAPHICS
// GLUT callback functions ---------------------------------------------------

// update the world - this is called whenever GLUT runs out of events
// to process
static void idle_func( void )
{
  Uni::UpdateAll();

  // possibly snooze to save CPU and slow things down
  if( Uni::sleep_msec > 0 )
    usleep( Uni::sleep_msec * 1e3 );
}

static void timer_func( int dummy )
{
  glutPostRedisplay(); // force redraw
}

// draw the world - this is called whenever the window needs redrawn
static void display_func( void ) 
{  
  if( Uni::need_redraw )
    {
      Uni::need_redraw = false;
      
      glClear( GL_COLOR_BUFFER_BIT );  
      
      FOR_EACH( r, population )
	r->Draw();
      
      glutSwapBuffers();
      
      glFlush();
    }
  
  // cause this run again in about 50 msec
  glutTimerFunc( 50, timer_func, 0 );
}

static void mouse_func(int button, int state, int x, int y) 
{  
  if( (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN ) )
    {
      Uni::paused = !Uni::paused;
    }
}

#endif // GRAPHICS



Robot::Robot()
  : pose(),
    speed(),
    color(),
    pixels( pixel_count ),
    callback(NULL),
    callback_data(NULL)
{
  // until C++ supports array literals in the initialization list, we're forced to do this
  memset( pose, 0, sizeof(pose));
  memset( speed, 0, sizeof(speed));
  color[0] = 128;
  color[1] = 0;
  color[2] = 0;
}


void Uni::Init( int argc, char** argv ) 
{
  // seed the random number generator with the current time
  // srand48(time(NULL));

  // seed the random number generator with a constant for repeatability
  srand48(0);

  bool quiet = false; // controls output verbosity

  int population_size = 100;
  // parse arguments to configure Robot static members
  // opterr = 0; // supress errors about bad options
  int c;  
  while( ( c = getopt( argc, argv, ":?dqp:s:f:r:c:u:z:w:")) != -1 )
    switch( c )
      {
      case 'p': 
	population_size = atoi( optarg );
	if( ! quiet ) fprintf( stderr, "[Uni] population_size: %d\n", population_size );
	population.resize( population_size );
	break;
	
      case 's': 
	worldsize = atoi( optarg );
	if( ! quiet ) fprintf( stderr, "[Uni] worldsize: %d\n", worldsize );
	break;
	
      case 'f': 
	Robot::fov = mdtomr( 1000 * atoi( optarg )); // degrees to milliradians
	if( ! quiet ) fprintf( stderr, "[Uni] fov: %d mrad\n", Robot::fov );
	break;
	
      case 'r': 
	Robot::range = atoi( optarg );
	if( ! quiet ) fprintf( stderr, "[Uni] range: %d\n", Robot::range );
	break;
	
      case 'c':
	Robot::pixel_count = atoi( optarg );
	if( ! quiet ) fprintf( stderr, "[Uni] pixel_count: %d\n", Robot::pixel_count );
	break;
				
      case 'u':
	updates_max = atol( optarg );
	if( ! quiet ) fprintf( stderr, "[Uni] updates_max: %lu\n", (long unsigned)updates_max );
	break;
				
      case 'z':
	sleep_msec = atoi( optarg );
	if( ! quiet ) fprintf( stderr, "[Uni] sleep_msec: %d\n", sleep_msec );
	break;
				
#if GRAPHICS
      case 'w': winsize = atoi( optarg );
	if( ! quiet ) fprintf( stderr, "[Uni] winsize: %d\n", winsize );
	break;

      case 'd': show_data= false;
	if( ! quiet ) fprintf( stderr, "[Uni] hide data\n" );
	break;

      case 'q': quiet = true;
	break;

#endif			
      case '?':
	puts( usage );
	exit(0); // ok
	break;

      default:
	fprintf( stderr, "[Uni] Option parse error.\n" );
	puts( usage );
	exit(-1); // error
      }
	
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
  glScalef( 1.0/worldsize, 1.0/worldsize, 1 ); 
  
  // define a display list for a robot body
  double h = 10;
  double w = 10;

  glPointSize( 4.0 );

  displaylist = glGenLists(1);
  glNewList( displaylist, GL_COMPILE );

  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

  glBegin( GL_POLYGON );
  glVertex2f( h/2.0, 0 );
  glVertex2f( -h/2.0,  w/2.0 );
  glVertex2f( -h/2.0, -w/2.0 );
  glEnd();

  glEndList();
#endif // GRAPHICS
  
  struct timeval start;
  gettimeofday( &start, NULL );
  lastseconds =  start.tv_sec + start.tv_usec/1e6;
}

void Robot::UpdateSensor()
{
  const int32_t mrad_per_pixel = fov / pixel_count;  
  const int32_t halfworld = worldsize / 2;
  
  // initialize pixels vector  
  FOR_EACH( it, pixels )
    {
      it->range = Robot::range; // maximum range
      it->robot = NULL; // nothing detected
    }
  
  // check every robot in the world to see if it is detected
  FOR_EACH( it, population )
    {
      Robot* other = &(*it);
      
      // discard if it's the same robot
      if( other == this )
	continue;
      
      // discard if it's out of range. We put off computing the
      // hypotenuse as long as we can, as it's relatively expensive.
      
      int32_t dx = other->pose[0] - pose[0];
      
      // wrap around torus
      if( dx > halfworld )
	dx -= worldsize;
      else if( dx < -halfworld )
	dx += worldsize;
      
      if( fabs(dx) > Robot::range )
	continue; // out of range
      
      int32_t dy = other->pose[1] - pose[1];
      
      // wrap around torus
      if( dy > halfworld )
	dy -= worldsize;
      else if( dy < -halfworld )
	dy += worldsize;
      
      if( fabs(dy) > Robot::range )
	continue; // out of range
      
      int32_t range = hypot( dx, dy );
      if( range > Robot::range ) 
	continue; 
      
      // discard if it's out of field of view 
      const int32_t absolute_heading = atan2( dy, dx ) * 1000;
      int32_t relative_heading = AngleNormalize((absolute_heading - pose[2]) );
      if( abs(relative_heading) > fov/2   ) 
	continue; 
      
      // find which pixel it falls in 
      relative_heading += fov/2; 
      const uint32_t pixel = (relative_heading / mrad_per_pixel) % pixel_count;
      
      assert( pixel >= 0 );
      assert( pixel < pixel_count );
      
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
  const int32_t dx = speed[0] * cos(pose[2]/1000.0);
  const int32_t dy = speed[0] * sin(pose[2]/1000.0); 
  const int32_t da = speed[1];
  
  pose[0] = DistanceNormalize( pose[0] + dx );
  pose[1] = DistanceNormalize( pose[1] + dy );
  pose[2] = AngleNormalize( pose[2] + da );
}

void Uni::UpdateAll()
{
  // if we've done enough updates, exit the program
  if( updates_max > 0 && updates > updates_max )
    {
      FOR_EACH( it, population )
	printf( "%d,%d,%d\n", it->pose[0], it->pose[1], it->pose[2] );
      exit(1);
    }
  
  if( ! paused )
    {   
      FOR_EACH( r, population )
	r->UpdatePose();
      
      FOR_EACH( r, population )
	r->UpdateSensor();
     
      FOR_EACH( r, population )
	{
	  Robot &b = *r;
	  r->callback( b, r->callback_data);
	}

      need_redraw = true;
    }
  
  const int period = 10;

  if( updates % period == 0 )
    {
      struct timeval now;
      gettimeofday( &now, NULL );
      double seconds = now.tv_sec + now.tv_usec/1e6;
      double interval = seconds - lastseconds;
      fprintf( stderr, "[%d] FPS %.3f\r",(int)updates, period/interval );      
      fflush(stdout);
      lastseconds = seconds;      
    }


  ++updates;  
}

// draw a robot
void Robot::Draw() const
{
#if GRAPHICS
  glPushMatrix();
  glTranslatef( pose[0], pose[1], 0 );
  glRotatef( mrtomd(pose[2])/1000.0, 0,0,1 );
  
  glColor3ub( color[0], color[1], color[2] ); 
  
  // draw the pre-compiled triangle for a body
  glCallList(displaylist);
  
  if( show_data )
    {
      // render the sensors
      double rad_per_pixel = (fov/1000.0) / pixel_count;
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      
      for( unsigned int p=0; p<pixel_count; p++ )
	{
	  double angle = -(fov/1000.0)/2.0 + (p+0.5) * rad_per_pixel;
	  double dx1 = pixels[p].range * cos(angle+rad_per_pixel/2.0);
	  double dy1 = pixels[p].range * sin(angle+rad_per_pixel/2.0);
	  double dx2 = pixels[p].range * cos(angle-rad_per_pixel/2.0);
	  double dy2 = pixels[p].range * sin(angle-rad_per_pixel/2.0);
	  
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





void Uni::Run()
{
#if GRAPHICS
  glutMainLoop();
#else
  while( 1 )
    {
      FOR_EACH( r, population )
	r->Update();
#endif
}

