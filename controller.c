/*****
		controller.c
		version 2
		Richard Vaughan
****/

#include "universe.h"

static int invert = 0;

// this is the robot controller code
void controller( robot_t* rob )
{
  // steer away from the closest roboot
  int closest = -1;
  double dist = rob->range;
  
  int p;
  for( p=0; p<rob->pixel_count; p++ )
    if( rob->pixels[p].range < dist )
      {
		  closest = p;
		  dist = rob->pixels[p].range;
      }
  
  // a very small chance of jumping heading, prevents any stable
  // patterns
  if( g_random_double() < 0.0001 ) 
	 { 
		rob->pose.a += g_random_double_range( -0.2, 0.2 ); 
	 } 
  else 
	 {
		rob->v_speed = 1.0;
		rob->w_speed = 0.001;
		
		if( closest < 0 ) // nothing nearby 
		  return;
		
		if( closest < rob->pixel_count / 2 )
		  rob->w_speed += +0.2;
		else
		  rob->w_speed += -0.2;
		
		if( invert )
		  rob->w_speed *= -1.0;
	 }
}

// program parameters
static int angle = 20; // robot sensor field of view in degrees // -a
static int range = 20; // robot sensor range // -r
static int pixels = 9; // robot sensor resolution // -x
static int population = 100; // -p
static int seconds = 10000;  // -s
static int updates_per_second = 5; // -u
static int msec = 10;  // -m
static int size = 100; // length of a world side // -s
static int data = 0; // visualize sensor data

/* THE OPTION-PARSING CODE REQUIRES GLIB 2.6 OR LATER */
static GOptionEntry entries[] =
  {
    { "angle", 'a', 0, G_OPTION_ARG_INT, &angle, "Robot sensor field of view in degrees", "20" },
    { "range", 'r', 0, G_OPTION_ARG_INT, &range, "Robot sensor range", "20" },
    { "pixels", 'x', 0, G_OPTION_ARG_INT, &pixels, "Robot sensor pixel count", "9" },
    { "size", 's', 0, G_OPTION_ARG_INT, &size, "World is square, with sides this long", "100" },
    { "population", 'p', 0, G_OPTION_ARG_INT, &population, "Size of robot population", "100" },
    { "time", 't', 0, G_OPTION_ARG_INT, &seconds, "Number of seconds to simulate", "10000" },
    { "updates-per-second", 'u', 0, G_OPTION_ARG_INT, &updates_per_second, "Number of simulation steps in one simulated second", "5" },
    { "msec", 'm', 0, G_OPTION_ARG_INT, &msec, "Sleep time between updates in milliseconds", "1" },
    { "invert", 'i', 0, G_OPTION_ARG_NONE, &invert, "Invert robot heading", NULL },
	 { "data", 'd', 0, G_OPTION_ARG_NONE, &data, "Disable sensor data visualization", NULL },
    { NULL }
  };

// processes the command line to set the parameters
void parse_args( int* argc, char* argv[] )
{
  GError *error = NULL;
  GOptionContext* context =
    g_option_context_new( "- test tree model performance" );
  g_option_context_add_main_entries( context, entries, NULL );
  g_option_context_parse( context, argc, &argv, &error );
}

int main( int argc, char* argv[] )
{
  parse_args( &argc, argv );

  // build a world, with our command line options
  world_t* world = world_create( &argc, argv,
											size, seconds, updates_per_second, msec, data );
  
  // add all the robots
  while( world->robot_count < population )
    world_add_robot( world, robot_create_random( world, 
																 1,
																 range,
																 DTOR(angle),
																 pixels,
																 controller ));
 
  // and start the simulation running
  world_run( world );
      
  // we'll never get here, but this keeps the compiler happy.
  return 0;
}
