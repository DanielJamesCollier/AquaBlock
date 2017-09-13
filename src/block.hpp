#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "material.hpp"

struct block {
   material type {material::AIR};
   float mass {0}; 
   float mass_next {0};

   void setGround() {
        type = material::GROUND;
        mass = 0;
        mass_next = 0;
   }

   void setAir() {
        type = material::AIR;
        mass = 0;
        mass_next = 0;
   }

   void setWater(float mass) {
        type = material::WATER;
        //mass = mass;
        mass_next = mass; 
   }
}; 
#endif // BLOCK_HPP
