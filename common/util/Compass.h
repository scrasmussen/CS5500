#ifndef COMPASS_H
#define COMPASS_H
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/concurrent_queue.h"
#include "../world.h"
#include "../voxel/chunkmanager.cpp"

class Compass
{
 public:
  glm::vec3 currentPos;
  int n;
  World world;
  ChunkManager chunk;

  Compass(World, ChunkManager, glm::vec3);
  tbb::concurrent_queue<glm::vec3> getQueue( glm::vec3&,int, int);
  glm::vec3 find(BlockType);

};
#endif
