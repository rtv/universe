/****
     universe.h
     Clone this package from git://github.com/rtv/universe.git
     version 2
     Richard Vaughan  
****/

#include <vector>
#include <queue>
#include <string>
#include <math.h> 
#include <stdio.h>
#include <stdlib.h>

#define GRAPHICS 1

// handy STL iterator macro pair. Use FOR_EACH(I,C){ } to get an iterator I to
// each item in a collection C.
#define VAR(V,init) __typeof(init) V=(init)
#define FOR_EACH(I,C) for(VAR(I,(C).begin());I!=(C).end();I++)

const int32_t THOUSANDPI = 3142; // ceil( M_PI * 1000 )

namespace Uni
{
  class Robot;
  
  /** initialization: call this before using any other calls. */	
  void Init( int argc, char** argv );

  /** update all robots */
  void UpdateAll();
  
  
  /** Start running the simulation. Does not return. */
  void Run();
  
  extern uint64_t updates; // number of simulation steps so far	 
  extern uint64_t updates_max; // number of steps to run before quitting (0 means infinity)
  extern int32_t worldsize; // side length of the toroidal world
  
  class Robot
  {
  public:
    // static data members (same for all instances)
    static int32_t range;      // sensor detects objects up tp this maximum distance
    static int32_t fov;        // sensor detects objects within this angular field-of-view (in milliradians)    
    static unsigned int  pixel_count; // number of pixels in sensor array
    
    // non-static data members
    int32_t pose[3] ;   // 2d pose and orientation [0]=x, [1]=y, [2]=a;
    int32_t speed[2];   // linear speed [0] in units per timestep and angular speed [1] in milliradians per timestep
    uint8_t color[3];  // body color [0]=red, [1]=green, [2]=blue;

    class Pixel 
    {
    public:
      int32_t range; // between zero and Robot::range
      Robot* robot; // closest robot detected or NULL if nothing detected
    };
    
    std::vector<Pixel> pixels; // sensor array
    
    // default constructor
    Robot( void );
    
    // default destructor
     ~Robot() {}
        
    // render the robot in OpenGL
    void Draw() const;
    
    // move the robot
    void UpdatePose();
    
    // update
    void UpdateSensor();
    
    // callback function for controlling this robot
    void (*callback)( Robot& r, void* user );
    void* callback_data;;    
  };	
  
  extern std::vector<Robot> population;
  
  // utilities
  
  /** Normalize a length to within 0 to worldsize. */
  inline int32_t DistanceNormalize( int32_t d )
  {
    while( d < 0 ) d += worldsize;
    while( d > worldsize ) d -= worldsize;
    return d; 
  } 
  
  /** Normalize an angle to within +/_ THOUSANDPI. */
  inline int32_t AngleNormalize( int32_t a )
  {
    while( a < -THOUSANDPI ) a += 2.0*THOUSANDPI;
    while( a >  THOUSANDPI ) a -= 2.0*THOUSANDPI;	 
    return a;
  }	 
  
  /** Convert radians to degrees. */
  inline int32_t mrtomd( int32_t mr )
  { return( mr * 180000 / THOUSANDPI ); }
  
  /** Convert degrees to radians */
  inline int32_t mdtomr( int32_t d)
  { return( d * THOUSANDPI / 180000); }
  
  inline void RandomPose( int32_t pose[3] )
  {
    pose[0] = drand48() * worldsize;
    pose[1] = drand48() * worldsize;
    pose[2] = AngleNormalize( drand48() * (THOUSANDPI*2.0));
  }    
    
    void Update();
    
    void Draw( void );
  
}; // namespace Uni
