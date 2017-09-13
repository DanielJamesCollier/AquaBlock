#include <iostream>
#include <cstdint>
#include <array>
#include <algorithm>                
#include <cmath>
#include <chrono>

#include "sdl_module.hpp"

/*
     controlls

    - left click  = place ground block
    - right click = place water block
    - space       = clear 
 */

template <typename T>
constexpr T
lerp(T v0, T v1, float t) {
    return (T(1) - t) * v0 + t * v1;
}


struct colour {

    colour(int rgb) 
    : r{rgb}
    , g{rgb}
    , b{rgb}{}

    colour(int r, int g, int b)
    :   r{r}
    ,   g{g}
    ,   b{b}
    {}
    
    int r;
    int g;
    int b;
};

colour lerp_colour(colour const & lhs, colour const & rhs, float amount) {
    colour colour(0,0,0);
    
    colour.r = lerp(lhs.r, rhs.r, amount);
    colour.g = lerp(lhs.g, rhs.g, amount);
    colour.b = lerp(lhs.b, rhs.b, amount);

    return colour;
}

enum class material : std::uint8_t {
   AIR    = 0,
   GROUND = 1,
   WATER  = 2, 
};

struct block {
   material     type {material::AIR};
   float mass {0}; 
   float mass_next {0};
}; 

int main() {
    constexpr int num_cols  = 50; // width 
    constexpr int num_rows  = 30;  // height 
    constexpr int cell_size = 20;
    constexpr int grid_size = num_cols * num_rows;
    constexpr int window_width = num_cols * cell_size;
    constexpr int window_height = num_rows * cell_size;
    constexpr float min_mass = 5;
    constexpr float max_mass = 255;
    colour min_mass_colour(94, 198, 255);
    colour max_mass_colour(0, 165, 255);
    colour max_water_colour(0, 72, 112);

    try {
        sdl_module sdl("AquaBlock", window_width, window_height);
        
        // data 
        std::array<block, grid_size> blocks{};
        std::array<std::uint32_t, grid_size> pixels;

        bool running = true; 
        int mouse_x = 0;
        int mouse_y = 0; 
        int mouse_grid_x = 0;
        int mouse_grid_y = 0;
        //..
        
        bool left_down = false;
        bool right_down = false; 
        
        // set top row to ground 
        for (int x = 0; x < num_cols; x++) {
            blocks[x].type = material::GROUND;
            blocks[x].mass = 0;
        }

        // set bottom row to ground 
        for (int x = grid_size - num_cols; x < grid_size; x++) {
            blocks[x].type = material::GROUND;
            blocks[x].mass = 0;
        }
        
        // set the left column to ground
        for (int y = 0; y < num_rows; y++) {
            int index = y * num_cols;
            blocks[index].type = material::GROUND;
            blocks[index].mass = 0;
        }
        
        // set the right column to ground 
        for (int y = 0; y < num_rows; y++) {
            int index = (num_cols - 1) + y * num_cols;
            blocks[index].type = material::GROUND;
            blocks[index].mass = 0;
        }

        // timing stuffs
        using namespace std::chrono_literals;
        auto frames = 0;
        auto start = std::chrono::system_clock::now();

        while(running) {
            auto now = std::chrono::system_clock::now();
            auto passed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            
            if (passed > 1s) {
                std::cout << "FPS: " << frames << std::endl;
                start = std::chrono::system_clock::now();
                frames = 0;
            }    

            frames++;
            
            auto clamp = [](auto const & val, auto const & min, auto const & max) {
                if (val > max) {
                    return max;
                } else if (val < min) {
                    return min;
                }
                return val;
            }; 

            // poll input      
            while (SDL_PollEvent(&sdl.m_event)) {
                switch (sdl.m_event.type) {
                    case SDL_QUIT:
                        running = false; 
                        break;
                    case SDL_MOUSEMOTION:
                    {
                        mouse_x = sdl.m_event.motion.x;
                        mouse_y = sdl.m_event.motion.y;
                        mouse_grid_x = mouse_x / cell_size;
                        mouse_grid_y = mouse_y / cell_size;
                        
                        // do not allow the edges to be selected 
                        mouse_grid_x = clamp(mouse_grid_x, 1, num_cols - 2);
                        mouse_grid_y = clamp(mouse_grid_y, 1, num_rows - 2);
                        
                        break;
                    }
                    case SDL_MOUSEBUTTONDOWN:
                        switch (sdl.m_event.button.button) {
                            case SDL_BUTTON_LEFT: 
                                left_down = true; 
                                break;
                            case SDL_BUTTON_RIGHT:
                                right_down = true;
                                break;
                        }
                        break;

                    case SDL_MOUSEBUTTONUP:
                        switch (sdl.m_event.button.button) {
                            case SDL_BUTTON_LEFT:
                                left_down = false;
                                break;
                            case SDL_BUTTON_RIGHT:
                                right_down = false;
                                break;
                        }
                        break;

                    case SDL_KEYDOWN:
                        switch(sdl.m_event.key.keysym.sym) {
                            case SDLK_SPACE:
                                for (auto x = 1; x < num_cols - 1; x++) {
                                    for (auto y = 1; y < num_rows -1; y++) {
                                        auto index = x + y * num_cols;
                                        blocks[index].type = material::AIR;
                                        blocks[index].mass = 0;
                                        blocks[index].mass_next = 0;
                                    }
                                }
                                break;
                            case SDLK_0:
                                for (auto x = 1; x < num_cols - 1; x++) {
                                    for (auto y = 1; y < num_rows - 1; y++) {
                                        auto& block = blocks[x + y * num_cols];

                                        if (block.type == material::WATER) {
                                            block.type = material::AIR;
                                            block.mass = 0;
                                            block.mass_next = 0;
                                        }
                                    }
                                }
                                break; // SDLK_0

                            case SDLK_1:
                            {
                                auto const & block = blocks[mouse_grid_x + mouse_grid_y * num_cols];
                                
                                std::cout << "\n---------\n";
                                std::cout << "material: " << (unsigned)block.type << "\nmass: " << +block.mass << "\nmass_next: " << +block.mass_next << "\n";
                            } break;
                        }
                        break; // SDL_KEYDOWN
                }
            }
            
            block & current_block = blocks[mouse_grid_x + mouse_grid_y * num_cols];

            if (left_down) {
                switch (current_block.type) {
                    case material::WATER: 
                        [[fallthrough]]; 
                    case material::AIR: 
                        current_block.type      = material::GROUND;
                        current_block.mass      = 0;
                        current_block.mass_next = 0;
                        break;
                }
            } 

            if (right_down) {
                current_block.type = material::WATER;
                current_block.mass = max_mass;
                current_block.mass_next = max_mass;
            }
            
           // render 
            {
                SDL_SetRenderDrawColor(sdl.m_renderer,76, 76, 76, 255); // background colour // aka air colour - probabaly should draw air blocks ?!
                sdl.clear_back_buffer();

                for (int y = 0; y < num_rows; y++) {
                    for (int x = 0; x < num_cols; x++) {
                        int index = x + y * num_cols;
                        auto& current_type = blocks[index].type; 

                        SDL_Rect r;

                        r.x = x * cell_size;
                        r.y = (y * cell_size) + cell_size; 
                        r.w = cell_size;
                        r.h = -cell_size;

                       
                        switch (current_type) {
                            case material::AIR:
                                break;
                            case material::GROUND:
                                SDL_SetRenderDrawColor(sdl.m_renderer, 24, 44, 76,255);
                                SDL_RenderFillRect(sdl.m_renderer, &r); 
                                break;

                            case material::WATER:
                            {
                                auto current_mass = blocks[index].mass;
                                auto above_mass   = blocks[x + (y - 1) * num_cols].mass;

                                colour mass_colour(lerp_colour(min_mass_colour, max_mass_colour, (float)blocks[index].mass / 255.0f));

                                if (!(above_mass != 0 && current_mass !=0)) { // if the block is falling
                                    r.h = -(float)cell_size * ((float)blocks[index].mass / 255.0f); // scale the box depending on the amount of water in the cell
                                }
                                
                                SDL_SetRenderDrawColor(sdl.m_renderer, mass_colour.r, mass_colour.g, mass_colour.b ,255);
                                SDL_RenderFillRect(sdl.m_renderer, &r); 
                                break;
                            }
                        } 
                    }
                }   

                // draw mouse position 
                SDL_Rect mouse_position;
                mouse_position.x = mouse_grid_x * cell_size;
                mouse_position.y = mouse_grid_y * cell_size;
                mouse_position.w = cell_size;
                mouse_position.h = cell_size;

                SDL_SetRenderDrawColor(sdl.m_renderer, 255, 0, 0, 255);
                SDL_RenderDrawRect(sdl.m_renderer, &mouse_position);

                sdl.back_to_front();
            }

            // update
            {
               
                for (int y = 0; y < num_rows; y++) {
                   for (int x = 0; x < num_cols; x++) {
                       
                        auto current_index = x + y * num_cols;
                        auto down_index = x + (y + 1) * num_cols;

                        auto remaining = blocks[current_index].mass;

                        if (remaining < min_mass) continue;
                       
                        if (blocks[current_index].type != material::WATER) continue;         
                        
                        
                        // downwards flow
                        if (blocks[down_index].type != material::GROUND) {
                            auto room = max_mass - blocks[down_index].mass;
                            auto flow = std::min(room, remaining);
                            
                            // if flow is negative constrain to 0
                            flow = std::max(flow, 0.0f);

                            if (flow > std::min(max_mass, blocks[current_index].mass))
        						flow = std::min(max_mass, blocks[current_index].mass);

                            blocks[current_index].mass_next -= flow;
                            blocks[down_index].mass_next += flow ;
                            remaining -= flow;
                        }   
                        
                        if (remaining < min_mass) continue; 
                        
                        auto left_index = (x - 1) + y * num_cols;
                        
                        // left flow
                        if (blocks[left_index].type != material::GROUND) {
                            
                            auto room = max_mass - blocks[left_index].mass;
                            auto flow = std::min(room, remaining) / 5.0f;
                            
                            flow = std::max(flow, 0.0f);

                            if (flow > std::min(max_mass, blocks[current_index].mass))
        						flow = std::min(max_mass, blocks[current_index].mass);

                            blocks[current_index].mass_next -= flow;
                            blocks[left_index].mass_next += flow;
                            remaining -= flow;
                        }
                        
                        if (remaining < min_mass) continue; 

                        auto right_index = (x + 1) + y * num_cols;
                        
                        // right flow
                        if (blocks[right_index].type != material::GROUND) {
                            auto room = max_mass - blocks[right_index].mass;
                            auto flow = std::min(room, remaining) / 4.0f;
                            
                            flow = std::max(flow, 0.0f);

                            if (flow > std::min(max_mass, blocks[current_index].mass))
        						flow = std::min(max_mass, blocks[current_index].mass);


                            blocks[current_index].mass_next -= flow;
                            blocks[right_index].mass_next += flow;
                            remaining -= flow;
                        }
                        
                        if (remaining < min_mass) continue; 
                   }
                }

                for (block & b : blocks) {
                    if (b.type == material::GROUND) continue;
                    b.mass = b.mass_next;

                    if (b.mass < min_mass) {
                        b.mass = 0;
                        b.mass_next = 0;
                        b.type = material::AIR;
                    } else {
                        b.type = material::WATER;
                        
                        // clamp the mass 
                        if (b.mass > max_mass) {
                            b.mass = max_mass;
                        }
                    }
                }
            }
        } 
    } catch (sdl_module_exception const &e) {
        std::cerr << e.what() << std::endl;

    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;

    } catch (...) {
    
    }

    return 0;
}
