#ifndef CHUNK_H
#define CHUNK_H

#include "block.h"
#include "vector3.h"
#include "graphics.h"

class Chunk
{
public:
  Chunk(int x, int y, int z);
  ~Chunk();

  void setAllBlocks(BlockType type);
  BlockType get(int x, int y, int z);
  void set(int x, int y, int z, BlockType type);
  Vector3 getPosition();
  void update();
  void render();

  static const int CHUNK_SIZE = 16;

private:
  // The blocks data
  Block*** mBlocks;
  int X; // World Chunk Position;
  int Y; // World Chunk Position;
  int Z; // World Chunk Position;
};

#endif
