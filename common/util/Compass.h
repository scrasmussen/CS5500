#ifndef COMPASS_H
#define COMPASS_H
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/concurrent_queue.h"
#include "../world.h"
#include "vector3.cpp"


class Compass
{
 public:
  glm::vec3 currentPos;
  int n;
  World world;

  Compass(World, glm::vec3);
  tbb::concurrent_queue<glm::vec3> getQueue( glm::vec3&,int);
  int find(BlockType);

  

};
#endif
