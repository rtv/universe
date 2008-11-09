/****
   universe.h
   version 2
   Richard Vaughan  
****/

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 

// Convert radians to degrees
#define RTOD(r) ((r) * 180.0 / M_PI)
// Convert degrees to radians
#define DTOR(d) ((d) * M_PI / 180.0)

// angle wrap-around constrains angle z in the range -PI to +PI
#define ANGLE_NORMALIZE(z) atan2(sin(z), cos(z))

typedef struct
{
  double x,y,a; // 2d position and orientation
} pose_t;

// each robot has an array of these to store its observations
typedef struct
{
  double range;
  int color;
} pixel_t;  

typedef struct
{
  GList* robots;      // list of pointers to robots
  unsigned long robot_count; // number of robots in list
  double seconds; // simulation time elapsed
  double seconds_max; // number of seconds to simulate
  double updates_per_second; // simulation time resolution
  int sleep_msec; // sleep for this many milliseconds per update cycle
  double size; // length of a side of the world, which is square.
} world_t;

typedef struct robot
{
  unsigned long id; // unique ID for this robot
  world_t* world;  // the world that contains this robot
  pose_t pose;     // robot is located at this pose
  int color;       // robot's body has this color
  double range;    // sensor detects objects up tp this maximum distance
  double fov;      // sensor detects objects within this angular
		   // field-of-view about the current heading
  double v_speed;  // current forward speed
  double w_speed;  // current angular speed (turn-rate)

  pixel_t* pixels; // robot's sensor vector
  size_t pixel_count; // length of the sensor vector

  void (*func)(struct robot *); // this function gets called to update the robot
} robot_t;


// user can pass a function of this type into universe_run() to
// control robots.
typedef void (*control_func_t)(robot_t*);

// create a new robot with these parameters
robot_t* robot_create( world_t* world,
		       double x, double y, double a, 
		       int color, 
		       double range,
		       double fov,
		       int pixel_count,
		       double v_speed,
		       double w_speed,
		       control_func_t func );

// create a new robot with random intitial pose and speeds
robot_t* robot_create_random( world_t* world,
			      int color,
			      double range,
			      double fov,
			      int pixel_count,
			      control_func_t func );

// create a new world with these parameters
world_t* world_create( int* argc,
		       char** argv,
		       double size, // world is size^2 units
		       double seconds, // number of seconds to simulate
		       int updates_per_second, // updates per timestep
		       int sleep_msec );

void world_add_robot( world_t* world, robot_t* robot );

void world_run( world_t* world );

