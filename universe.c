/****
   universe.c
   version 2
   Richard Vaughan  
****/

#include <assert.h>
#include <unistd.h>

#include "universe.h"

#define GRAPHICS 1

#if GRAPHICS
 #include <GLUT/glut.h> // OS X users need <glut/glut.h> instead
 #define REDRAW_INTERVAL 1
 #define DRAW_SENSORS 1
 #define WIN_SIZE 500

 static int displaylist=0;
#endif

#define PROGNAME "universe"


// creates a new robot configured using the parameters, with initially
// zero speed. Returns a pointer to the new robot.
robot_t* robot_create( world_t* world,
		       double x, double y, double a, 
		       int color, 
		       double range,
		       double fov,
		       int pixel_count,
		       double v_speed,
		       double w_speed,
		       control_func_t func )
{
  static unsigned long id=0; // initialize ID counter

  robot_t* rob = calloc( 1, sizeof(robot_t) );
  rob->id = id++; // assign unique ID to this robot
  rob->world = world;
  rob->pose.x = x;
  rob->pose.y = y;
  rob->pose.a = a;
  rob->color = color;
  rob->range = range;
  rob->fov = fov;
  rob->func = func;
  rob->v_speed = v_speed;
  rob->w_speed = w_speed;
  
  // initialize pixelss
  rob->pixels = calloc( sizeof(pixel_t), pixel_count );  
  rob->pixel_count = pixel_count;
  int p;
  for( p=0; p<pixel_count; p++ )
    {
      rob->pixels[p].range = rob->range;
      rob->pixels[p].color = 0;
    }
          
  return rob;
}

// Creates a new robot using the parameters, with initially zero speed
// and random pose in the range 0-1 in X and Y, and 0-2PI in orientation,
robot_t* robot_create_random( world_t* world,
			      int color, 
			      double range,
			      double fov,
			      int pixels,
			      control_func_t func )
{
  double x = g_random_double_range( 0.0, world->size );
  double y = g_random_double_range( 0.0, world->size );
  double a = g_random_double_range( 0.0, 2.0 * M_PI );
  
  double v_speed = g_random_double_range( 0, 10 );
  double w_speed = g_random_double_range( 0, 1.0 );

  // randomly invert the direction of the turning speed
  if( g_random_double() > 0.5 )
    w_speed -= w_speed;
  
  return robot_create( world, 
		       x, y, a, 
		       color, 
		       range, fov, pixels,
		       v_speed, w_speed,
		       func );
}

void robot_destroy( robot_t* rob )
{
  if( rob->pixels )
    free( rob->pixels);

  free( rob );
}
 
void robot_callback_wrapper( robot_t* rob, void (*user)(robot_t*) )
{
  (*user)( rob );
}

void robot_print( robot_t* rob )
{
  printf( "%.2f %lu\t%.2f\t%.2f\t%.2f\t( %d %.2f %.2f )\t",
	  rob->world->seconds,
	  rob->id,
	  rob->pose.x,
	  rob->pose.y,
	  rob->pose.a,
	  rob->color,
	  rob->range,
	  rob->fov );
  
  int p;
  for( p=0; p<rob->pixel_count; p++ )
    printf( "[%d %.2f] ",
	    rob->pixels[p].color,
	    rob->pixels[p].range );
  puts("");
}

void robot_print_pose( robot_t* rob )
{
  printf( "%lu\t%.2f\t%.2f\t %.2f\n",
	  rob->id,
	  rob->pose.x,
	  rob->pose.y,
	  rob->pose.a );
}

double length_normalize( double z, double zmax )
{
  while( z < 0 ) z += zmax;
  while( z > zmax ) z -= zmax;
  return z; 
} 

double angle_normalize( double a )
{
  while( a < -M_PI ) a += 2.0*M_PI;
  while( a >  M_PI ) a -= 2.0*M_PI;	 
  return a;
}

void robot_update_pose( robot_t* rob )
{
  double stepsize = 1.0 / rob->world->updates_per_second; 
  double dx = stepsize * rob->v_speed * cos(rob->pose.a);
  double dy = stepsize *  rob->v_speed * sin(rob->pose.a);; 
  double da = stepsize * rob->w_speed;

  rob->pose.x = length_normalize( rob->pose.x + dx, rob->world->size );
  rob->pose.y = length_normalize( rob->pose.y + dy, rob->world->size );
  rob->pose.a = angle_normalize( rob->pose.a + da );
}
  
void robot_update_pixels( robot_t* rob )
{
  double radians_per_pixel = rob->fov / (double)rob->pixel_count;

  // initialize pixels vector
  int p;
  for( p=0; p<rob->pixel_count; p++ )
    {
      rob->pixels[p].range = rob->range; 
      rob->pixels[p].color = 0; // indicates nothing detected
    }

  // check every other robot in the world
  GList* it;
  for( it = rob->world->robots; it; it=it->next )
    {
      robot_t* other = (robot_t*)it->data;
      
      // discard if it's the same robot
      if( rob == other )
	continue;

      // discard if it's out of range
      double dx = rob->pose.x - other->pose.x;
      double dy = rob->pose.y - other->pose.y;
      double range = hypot( dx, dy );
      if( range > rob->range ) 
 	continue; 
      
      // discard if it's out of field of view 
      double absolute_heading = atan2( dy, dx );
      double relative_heading = angle_normalize((absolute_heading - rob->pose.a) + M_PI );
      if( fabs(relative_heading) > rob->fov/2.0   ) 
	continue; 

      // find which pixel it falls in 
      int pixel = (int)
	(relative_heading / radians_per_pixel);
      pixel += rob->pixel_count / 2;
      pixel %= rob->pixel_count;

      if( pixel < 0 || pixel >=rob->pixel_count )
	continue;

      assert( pixel >= 0 );
      assert( pixel < rob->pixel_count );
      
      // discard if we've seen something closer in this pixel already.
      if( rob->pixels[pixel].color && 
	  (rob->pixels[pixel].range < range) )
	continue;
      
      // if we made it here, we see this other robot in this pixel.
      rob->pixels[pixel].range = range;
      rob->pixels[pixel].color = other->color;
    }	
}

void robot_update( robot_t* rob )
{
  //puts( "robot update" );

  robot_update_pixels( rob );
  
  if( rob->func )
    (*rob->func)(rob);

  robot_update_pose( rob );
}


void world_update( world_t* world )
{
  //puts( "world update" );
  
  if( world->sleep_msec > 0 )
    usleep( world->sleep_msec * 1e3 );

  // test to see if we've finished running
  if( world->seconds > world->seconds_max )
    {
      fflush(stdout);
      exit( 0 ); // just dump out - quick and easy
    }

  // calculate a new world state
  g_list_foreach( world->robots, (GFunc)robot_callback_wrapper, robot_update ); 

  // use this to print logs of the robot state on stdout
  //g_list_foreach( world->robots, (GFunc)robot_callback_wrapper, robot_print );

  world->seconds += 1.0/world->updates_per_second;
}



#if GRAPHICS

world_t* glut_world;

// draw a robot
void robot_draw( robot_t* rob )
{
  glPushMatrix();
  glTranslatef( rob->pose.x, rob->pose.y, 0 );
  glRotatef( RTOD(rob->pose.a), 0,0,1 );

  glCallList(displaylist);

  if( glut_world->data )
	 {
		// render the sensors
		double rads_per_pixel = rob->fov / (double)rob->pixel_count;
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		
		int p; 
		for( p=0; p<rob->pixel_count; p++ )
		  {
			 double angle = -rob->fov/2.0 + (p+0.5) * rads_per_pixel;
			 double dx1 = rob->pixels[p].range * cos(angle+rads_per_pixel/2.0);
			 double dy1 = rob->pixels[p].range * sin(angle+rads_per_pixel/2.0);
			 double dx2 = rob->pixels[p].range * cos(angle-rads_per_pixel/2.0);
			 double dy2 = rob->pixels[p].range * sin(angle-rads_per_pixel/2.0);
			 
			 glColor4f( 1,0,0, rob->pixels[p].color ? 0.2 : 0.05 );
			 
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
  world_update( glut_world );
}

void timer_func( int dummy )
{
  glutPostRedisplay(); // force redraw
}

// draw the world - this is called whenever the window needs redrawn
void display_func( void ) 
{
  
  glClear( GL_COLOR_BUFFER_BIT );
  
  g_list_foreach( glut_world->robots, 
						(GFunc)robot_callback_wrapper, 
						robot_draw );  

  glutSwapBuffers();

  glutTimerFunc( 50, timer_func, 0 );
}


#endif


world_t* world_create( int* argc,
							  char** argv,
							  double size, // area of the world
							  double seconds, // number of seconds to simulate
							  int updates_per_second, // updates per timestep
							  int sleep_msec, // sleep time per cycle
							  int data ) //toggle data visualization
{
  world_t* world = calloc( sizeof(world_t), 1 );
  world->seconds = 0;
  world->seconds_max = seconds;
  world->updates_per_second = updates_per_second;
  world->sleep_msec = sleep_msec;
  world->size = size;
  world->data = data;

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

  return world;
}		       

void world_destroy( world_t* world )
{
  GList* it;
  for( it = world->robots; it; it=it->next )
    robot_destroy( (robot_t*)it->data );

  g_list_free( world->robots );
  
  free( world );
}

void world_add_robot( world_t* world, robot_t* robot )
{
  world->robots = 
    g_list_prepend( world->robots, robot );

  world->robot_count++;
}

void world_run( world_t* world )
{
#if GRAPHICS
  glutMainLoop();
#else
  while( 1 )
    world_update( world );
#endif
}




