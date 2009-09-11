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
  
  Swarmer() : Robot( Pose::Random(), Color(0,0,1) )
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

int main( int argc, char* argv[] )
{
	// configure global robot settings
	Robot::Init( argc, argv );
	
  // parse remaining cmdline arguments to configure swarmer
	int c;
	while( ( c = getopt( argc, argv, "i")) != -1 )
	 	switch( c )
	 		{
	 		case 'i': Swarmer::invert = true;
	 			break;				
			}
	
	for( unsigned int i=0; i<Robot::population_size; i++ )
		new Swarmer();
	
	// and start the simulation running
	Robot::Run();
	
	// we'll probably never get here, but this keeps the compiler happy.
	return 0;
 }
