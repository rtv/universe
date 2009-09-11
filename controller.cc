/*****
		controller.cc
		version 1
		Copyright Richard Vaughan, 2009.09.09
****/

#include "universe.h"
using namespace Uni;

// this is the robot controller code
class Swarmer : public Robot
{
public:
  
  static bool invert;
  
  Swarmer() : Robot( Pose::Random(), Color(1,0,0) )
  {}
  
  // must implement this method. Examine the pixels vector and set the
  // speed sensibly.
  virtual void Controller()
  {
	 speed.v = 0.005;   // constant forward speed 
	 speed.w = 0.0;     // no turning. we may change this below
	 
	 // steer away from the closest roboot
	 int closest = -1;
	 double dist = Robot::range; // max sensor range
	 
	 for( unsigned int p=0; p<pixel_count; p++ )
		if( pixels[p].range < dist )
		  {
				closest = (int)p;
			 dist = pixels[p].range;
		  }
	 
	 if( closest < 0 ) // nothing nearby: cruise
		 return;
	 
	 if( closest < (int)pixel_count / 2 )
		 speed.w = 0.03; 
	 else
		 speed.w = -0.03; // turn left
	 
	 if( invert )
		 speed.w *= -1.0; // invert turn direction
	}	
};

// static members
bool Swarmer::invert( false );

  
// program parameters
// static int population = 100; // -p
// static int updates = 10000;  // -s
// static int updates_per_second = 5; // -u
// static int msec = 10;  // -m
// static int data = 0; // visualize sensor data

// /* THE OPTION-PARSING CODE REQUIRES GLIB 2.6 OR LATER */
// static GOptionEntry entries[] =
//   {
//     { "angle", 'a', 0, G_OPTION_ARG_INT, &Robot::fov, "Robot sensor field of view in degrees", "20" },
//     { "range", 'r', 0, G_OPTION_ARG_INT, &Robot::range, "Robot sensor range", "20" },
//     { "pixels", 'x', 0, G_OPTION_ARG_INT, &Robot::pixel_count, "Robot sensor pixel count", "9" },
//     { "steps", 's', 0, G_OPTION_ARG_INT, &Robot::updates_max, "Number of seconds to simulate", "10000" },
//     { "msec", 'm', 0, G_OPTION_ARG_INT, &Robot::sleep, "Sleep time between updates in milliseconds", "1" },

//     { "population", 'p', 0, G_OPTION_ARG_INT, &population, "Size of robot population", "100" },
//     { "invert", 'i', 0, G_OPTION_ARG_NONE, &invert, "Invert robot heading", NULL },
// 	 { "data", 'd', 0, G_OPTION_ARG_NONE, &data, "Disable sensor data visualization", NULL },
//     { NULL }
//   };

// // processes the command line to set the parameters
// void parse_args( int* argc, char* argv[] )
// {
//   GError *error = NULL;
//   GOptionContext* context =
//     g_option_context_new( "- test tree model performance" );
//   g_option_context_add_main_entries( context, entries, NULL );
//   g_option_context_parse( context, argc, &argv, &error );
// }



int main( int argc, char* argv[] )
 {
	int population = 400;

	// configure global robot settings
	Robot::Init( argc, argv );
	
	for( int i=0; i<population; i++ )
	  new Swarmer();
	
	// and start the simulation running
	Robot::Run();
	
	// we'll probably never get here, but this keeps the compiler happy.
	return 0;
 }
