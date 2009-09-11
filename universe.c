/****
   universe.c
   version 2
   Richard Vaughan  
****/

#include <assert.h>
#include <unistd.h>

#include "universe.h"
using namespace "Uni";

#define GRAPHICS 1

#if GRAPHICS
 #include <GLUT/glut.h> // OS X users need <glut/glut.h> instead
 #define REDRAW_INTERVAL 1
 #define DRAW_SENSORS 1
 #define WIN_SIZE 500

 static int displaylist=0;
#endif

#define PROGNAME "universe"

long unsigned int Robot::ids = 0;

// creates a new robot configured using the parameters, with initially
// zero speed. Returns a pointer to the new robot.
Robot::Robot( const World& world,
				  const Pose& pose, 
				  const Speed& speed,
				  int color, 
				  double range,
				  double fov,
				  int pixel_count )
:  id( ids++ )
   world(world),
	pose(pose),
	speed(speed),
	color(color),
	range(range),
	fov(fov),
	pixels( pixel_count )
{}


// Creates a new robot using the parameters, with initially zero speed
// and random pose in the range 0-1 in X and Y, and 0-2PI in orientation,
void World::AddRobotRandom( int color, 
									 double range,
									 double fov,
									 int pixel_count )
{
  Pose p( g_random_double_range( 0.0, size ),
			 g_random_double_range( 0.0, size );
			 g_random_double_range( 0.0, 2.0 * M_PI ) );
  
  Speed s( g_random_double_range( 0, 10 ),
			  g_random_double_range( -1.0, 1.0 ) );
  
  AddRobot( p, g,
				color, range, fov, pixel_count );
}

 
void Robot::UpdatePixels()
{
  double radians_per_pixel = fov / (double)pixels.size();
  
  // initialize pixels vector  
  const int pixel_count = pixels.size();

  for( int p=0; p<pixel_count; p++ )
    {
      pixels[p].range = range; 
      pixels[p].color = 0; // indicates nothing detected
    }
  
  // check every other robot in the world  
  for( int i=0; i < world.robots.size(); ++i )
    {
      Robot& other = world.robots[i];
      
      // discard if it's the same robot
      if( rob.id == other.id )
		  continue;
		
      // discard if it's out of range
      double dx = pose.x - other.pose.x;
      double dy = pose.y - other.pose.y;
      double range = hypot( dx, dy );
      if( range > this->range ) 
		  continue; 
		
      // discard if it's out of field of view 
      double absolute_heading = atan2( dy, dx );
      double relative_heading = AngleNormalize((absolute_heading - pose.a) + M_PI );
      if( fabs(relative_heading) > fov/2.0   ) 
		  continue; 
		
      // find which pixel it falls in 
      int pixel = (int)
		  (relative_heading / radians_per_pixel);
      pixel += pixel_count / 2;
      pixel %= pixel_count;

      if( pixel < 0 || pixel >= pixel_count )
		  continue;
		
      assert( pixel >= 0 );
      assert( pixel < pixel_count );
      
      // discard if we've seen something closer in this pixel already.
      if( pixels[pixel].color && 
			 ( pixels[pixel].range < range) )
		  continue;
      
      // if we made it here, we see this other robot in this pixel.
      pixels[pixel].range = range;
      pixels[pixel].color = other.color;
    }	
}

void Robot::Update()
{
  UpdatePixels();
  
  // run the virtual controller method
  Controller();
  
  // move according to the speed setting
  double stepsize = 1.0 / world.updates_per_second; 
  double dx = stepsize * speed.v * cos(pose.a);
  double dy = stepsize *  speed.v * sin(pose.a);; 
  double da = stepsize * speed.w;
  
  pose.x = length_normalize( pose.x + dx, world.size );
  pose.y = length_normalize( pose.y + dy, world.size );
  pose.a = angle_normalize( pose.a + da );
}


void World::Update()
{
  if( sleep_msec > 0 )
    usleep( sleep_msec * 1e3 );
  
  // test to see if we've finished running
  if( seconds > seconds_max )
    {
      fflush(stdout);
      exit( 0 ); // just dump out - quick and easy
    }

  // calculate a new world state
  for(std::vector<Robot>::iterator it = robots.begin();
		it != robots.end();
		++it )
	 it->Update();
  
  seconds += 1.0/updates_per_second;
}



#if GRAPHICS

world_t* glut_world;

// draw a robot
void Robot::Draw()
{
  glPushMatrix();
  glTranslatef( pose.x, pose.y, 0 );
  glRotatef( RTOD(pose.a), 0,0,1 );
  
  glCallList(displaylist);
  
  if( glut_world->data )
	 {
		// render the sensors

		int count = pixels.size();
		double rads_per_pixel = fov / (double)count;
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		
		for( int p=0; p<count; p++ )
		  {
			 double angle = -fov/2.0 + (p+0.5) * rads_per_pixel;
			 double dx1 = pixels[p].range * cos(angle+rads_per_pixel/2.0);
			 double dy1 = pixels[p].range * sin(angle+rads_per_pixel/2.0);
			 double dx2 = pixels[p].range * cos(angle-rads_per_pixel/2.0);
			 double dy2 = pixels[p].range * sin(angle-rads_per_pixel/2.0);
			 
			 glColor4f( 1,0,0, pixels[p].color ? 0.2 : 0.05 );
			 
			 glBegin( GL_POLYGON );
			 glVertex2f( 0,0 );
			 glVertex2f( dx1, dy1 );
			 glVertex2f( dx2, dy2 );
			 glEnd();                  
		  }	  
	 }

  glPopMatrix();
}


// update the world - this is called whenever GLUT runs out of events
// to process
void idle_func( void )
{
  glut_world->Update();
}

void timer_func( int dummy )
{
  glutPostRedisplay(); // force redraw
}

// draw the world - this is called whenever the window needs redrawn
void display_func( void ) 
{  
  glClear( GL_COLOR_BUFFER_BIT );  
  glut_world->Draw();
  glutSwapBuffers();

  glutTimerFunc( 50, timer_func, 0 );
}


#endif


World::World( int* argc,
				  char** argv,
				  double size, // area of the world
				  double seconds, // number of seconds to simulate
				  int updates_per_second, // updates per timestep
				  int sleep_msec, // sleep time per cycle
				  int data ) //toggle data visualization
: seconds(0),
  seconds_max(seconds),
  updates_per_second(updates_per_second),
  sleep_msec(sleep_msec),
  size(size),
  data(data)
{
#if GRAPHICS
  // initialize opengl graphics
  glut_world = world; // nasty hack to get around GLUT's stupidity
  glutInit( argc, argv );
  glutInitWindowSize( WIN_SIZE, WIN_SIZE );
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );
  glutCreateWindow( "Universe" );
  glClearColor( 0.8,0.8,1.0,1.0 );
  glutDisplayFunc( display_func );
  glutTimerFunc( 50, timer_func, 0 );
  glutIdleFunc( idle_func );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_BLEND );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0,1,0,1 );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glScalef( 1.0/size, 1.0/size, 1 ); 
  
  // define a display list for a robot body
  double h = 3;
  double w = 2;
  displaylist = glGenLists(1);
  glNewList( displaylist, GL_COMPILE );
  glColor4f( 0,0,1,1 );
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glBegin( GL_POLYGON );
  glVertex2f( h/2.0, 0 );
  glVertex2f( -h/2.0,  w/2.0 );
  glVertex2f( -h/2.0, -w/2.0 );
  glEnd();
  glEndList();
#endif
}		       


void World::Draw()
{
  for(std::vector<Robot>::iterator it = robots.begin();
		it != robots.end();
		++it )
	 it->Draw();
}

void World::Run( world_t* world )
{
#if GRAPHICS
  glutMainLoop();
#else
  while( 1 )
    Update();
#endif
}




