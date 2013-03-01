/*****
		controller.cc
		version 3
		Copyright Richard Vaughan, 2013.1.10
****/

#include "universe.h"

static bool invert = false;

// Examine the robot's pixels vector and set the speed sensibly.
void Controller( Uni::Robot& r, void* dummy_data )
{ 
  r.speed[0] = 0.005;   // constant forward speed 
  r.speed[1] = 0.0;     // no turning. we may change this below
  
  // steer away from the closest roboot
  int closest = -1;
  double dist = r.range; // max sensor range
  
  const size_t pixel_count = r.pixels.size();
  for( unsigned int p=0; p<pixel_count; p++ )
    if( r.pixels[p].range < dist )
      {
	closest = (int)p;
	dist = r.pixels[p].range;
      }
  
  if( closest < 0 ) // nothing nearby: cruise
    return;
  
  if( closest < (int)pixel_count / 2 )
    r.speed[1] = 0.04; // rotate right
  else
    r.speed[1] = -0.04; // rotate left
  
  if( invert )
    r.speed[1] *= -1.0; // invert turn direction	
}

int main( int argc, char* argv[] )
{
  // configure global robot settings
  Uni::Init( argc, argv );
  
  // parse remaining cmdline arguments to configure swarmer
  int c=0;
  while( ( c = getopt( argc, argv, "i")) != -1 )
    switch( c )
      {
      case 'i': invert = true;
	puts( "[Ctrl] invert" );
	break;				
      }
  
  // configure the robots the way I want 'em
  FOR_EACH( r, Uni::population )
    {
      Uni::RandomPose( r->pose ); 
      
      // install our callback function
      r->callback = Controller;
      r->callback_data = NULL;
    }
  
  // and start the simulation running
  Uni::Run();
  
  // we'll probably never get here, but this keeps the compiler happy.
  return 0;
}
