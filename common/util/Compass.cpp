#include "Compass.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/concurrent_queue.h"
#include "../world.h"
#include "chunkmanager.h"
#include <iostream>

int modx(int s)
{
  return (s>0?s%ChunkManager::BOUNDX :(s+ChunkManager::BOUNDX)%ChunkManager::BOUNDX);
};

int mody(int s)
{
  return (s>0?s%ChunkManager::BOUNDY :(s+ChunkManager::BOUNDY)%ChunkManager::BOUNDY);
};

int modz(int s)
{
  return s;
};

Compass::Compass(World inWorld, ChunkManager inChunk, glm::vec3 pos)
{
  world = inWorld;
  chunk = inChunk;
  currentPos = pos;
};

struct CheckType
{
  std::vector<std::shared_ptr<Block> > blocks;
  BlockType type;
  bool* found;
  ChunkManager* chunk;
  glm::vec3* distance;
  void operator()( const tbb::blocked_range<int>& range ) const
  {
    for (auto i=range.begin(); i!=range.end(); ++i)
      {
	if (type == chunk->get(blocks[i]->position.x, blocks[i]->position.y, blocks[i]->position.z))
	  {
	    (*found) = true;
	    (*distance) = blocks[i]->position;
	  }
      }
  }
};


glm::vec3 Compass::find(BlockType type)
{  
  auto blocks = world.blocks;
  auto worldSize = blocks.size();
  auto found = false;
  auto distance = 1;
  glm::vec3 typeDistance;

  while (distance <= static_cast<int>(worldSize)/2)
    {
      auto q = getQueue(currentPos, distance, worldSize);
      CheckType check;
      check.chunk =& chunk;
      check.blocks = blocks;
      check.type = type;
      check.found = &found;
      check.distance = &typeDistance;

      tbb::parallel_for( tbb::blocked_range<int>(0, q.unsafe_size()), check);
      if (found)
	return typeDistance;

       ++distance;
    }

  return glm::vec3(0,0,0);
};

struct populatej {
  tbb::concurrent_queue<glm::vec3>* queue;
  glm::vec3*  start;
  int distance;
  int i;
  int worldSize;
  void operator()( tbb::blocked_range<int>& range ) const {
    for( int j=range.begin(); j!=range.end(); ++j ){
      int k=i>0?distance-j:distance+j;
      queue->push(glm::vec3(start->x+i,start->y+j,start->z+k));
      if(k!=0)queue->push(glm::vec3(start->x+i,start->y+j,start->z-k));
    }
  }
};

struct populateQueue {
  tbb::concurrent_queue<glm::vec3>* queue;
  glm::vec3* start;
  int distance;
  int worldSize;
  void operator()( tbb::blocked_range<int>& range ) const {
    for( int i=range.begin(); i!=range.end(); ++i ){
      populatej jloop;
      int jdistance=i>0?distance-i:distance+i;
      jloop.distance=jdistance;
      jloop.queue=queue;
      jloop.start=start;
      jloop.i=i;
      jloop.worldSize=worldSize;
      tbb::parallel_for( tbb::blocked_range<int>(-jdistance,jdistance),jloop);
    }
  }
};

tbb::concurrent_queue<glm::vec3> Compass::getQueue( glm::vec3&  start,int dis , int worldSize) {
  tbb::concurrent_queue<glm::vec3> queue;
  populateQueue pq;
  pq.queue=&queue;
  pq.distance=dis;
  pq.worldSize=worldSize;
  pq.start=&start;
  tbb::parallel_for( tbb::blocked_range<int>( -dis, +dis ), pq );
  return queue;
}

