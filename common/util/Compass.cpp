#include "Compass.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/concurrent_queue.h"
#include "../world.h"

#include <array>
#include <iostream>
#include <typeinfo>

Compass::Compass(World input, glm::vec3 pos)
{
  world = input;
  currentPos = pos;
};

struct CheckType
{
  //const Block* input;
  std::vector<std::shared_ptr<Block> > blocks;
  glm::vec3 position;
  BlockType type;
  bool* output;
  void operator()( const tbb::blocked_range<int>& range ) const
  {
        for (int i=range.begin(); i!=range.end(); ++i)
      {
	    if (blocks[i]->position == position)
	      {
		if (blocks[i]->type == type)
		  output[i] = true;
		else
		  output[i] = false;
	  }
      }
  }
};


int Compass::find(BlockType type)
{  
  auto blocks = world.blocks;
  auto worldSize = blocks.size();
  auto found = false;
  auto distance = 1;
  bool boolArray[worldSize];

  while (!found)
    {
      auto q = getQueue(currentPos, distance);
      auto qSize = q.unsafe_size();
      glm::vec3 e;      

      while (q.try_pop(e))
	{
	  CheckType check;
	  check.blocks = blocks;
	  check.position = e;
	  check.type = type;
	  check.output = boolArray;

	  tbb::parallel_for( tbb::blocked_range<int>( 1, worldSize), check);

	  for (unsigned int i=0; i< worldSize; ++i)
	    if (boolArray[i] == true)
	      return distance;
	}

      if (worldSize == qSize)
	return -1;
      distance++;
    }
  
  return -1;
};

struct populatej {
  tbb::concurrent_queue<glm::vec3>* queue;
  glm::vec3*  start;
  int distance;
  int i;
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
  void operator()( tbb::blocked_range<int>& range ) const {
    for( int i=range.begin(); i!=range.end(); ++i ){
      populatej jloop;
      int jdistance=i>0?distance-i:distance+i;
      jloop.distance=jdistance;
      jloop.queue=queue;
      jloop.start=start;
      jloop.i=i;
      tbb::parallel_for( tbb::blocked_range<int>(-jdistance,jdistance),jloop);
    }
  }
};


tbb::concurrent_queue<glm::vec3> Compass::getQueue( glm::vec3&  start,int dis ) {
  tbb::concurrent_queue<glm::vec3> queue;
  populateQueue pq;
  pq.queue=&queue;
  pq.distance=dis;
  pq.start=&start;
  tbb::parallel_for( tbb::blocked_range<int>( -dis, +dis ), pq );
  return queue;
}

