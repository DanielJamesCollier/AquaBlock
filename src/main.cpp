// std
#include <iostream>
#include <cstdint>
#include <array>
#include <algorithm>                
#include <cmath>
#include <chrono>

// my
#include "sdl_module.hpp"
#include "colour.hpp"
#include "block.hpp"
#include "material.hpp"
#include "constants.hpp"

/*
    ------------controls -------------
    - left click  = place ground block
    - right click = place water block
    - space       = clear 
    - 0           = clear water only
    - 1           = print block info to console of red box selected block
 */


int main() {
    constexpr int num_cols  = 50;   
    constexpr int num_rows  = 30;   
    constexpr int cell_size = 20;
    constexpr int grid_size = num_cols * num_rows;
    constexpr int window_width = num_cols * cell_size;
    constexpr int window_height = num_rows * cell_size;

    auto clamp = [](auto const & val, auto const & min, auto const & max) {
        if (val > max) {
            return max;
        } else if (val < min) {
            return min;
        }
        return val;
    }; 
    
    try {
        sdl_module sdl("AquaBlock", window_width, window_height);
        
        std::array<block, grid_size> blocks{};
        std::array<std::uint32_t, grid_size> pixels;

        int mouse_x = 0;
        int mouse_y = 0; 
        int mouse_grid_x = 1;
        int mouse_grid_y = 1;
        
        bool left_down = false;
        bool right_down = false; 
        bool running = true; 
        
        // set top row to ground blocks
        for (auto x = 0; x < num_cols; x++) {
            blocks[x].setGround();
        }

        // set bottom row to ground blocks 
        for (auto x = grid_size - num_cols; x < grid_size; x++) {
            blocks[x].setGround();
        }
        
        // set the left column to ground blocks
        for (int y = 0; y < num_rows; y++) {
            blocks[y * num_cols].setGround();
        }
        
        // set the right column to ground blocks 
        for (int y = 0; y < num_rows; y++) {
            blocks[(num_cols - 1) + y * num_cols].setGround();
        }

        // fps counter display timer 
        using namespace std::chrono_literals;
        auto frames = 0;
        auto start = std::chrono::system_clock::now();

        while(running) {
            auto now = std::chrono::system_clock::now();
            auto passed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            
            // code inside if statement is ran every 1 second        
            if (passed > 1s) {
                std::cout << "FPS: " << frames << std::endl;
                start = std::chrono::system_clock::now();
                frames = 0;
            }    
            frames++;
            
            // poll input      
            while (SDL_PollEvent(&sdl.m_event)) {
                switch (sdl.m_event.type) {
                    
                    case SDL_QUIT:
                        running = false; 
                        break;
                    
                    case SDL_MOUSEMOTION:
                    {
                        // get the real mouse position and the mouse position within the grid
                        mouse_x = sdl.m_event.motion.x;
                        mouse_y = sdl.m_event.motion.y;
                        mouse_grid_x = mouse_x / cell_size;
                        mouse_grid_y = mouse_y / cell_size;
                        
                        // the ground boarder around the window should not be selectable so clamp it 
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
                                // set all blocks to air - ignore the ground block border
                                for (auto x = 1; x < num_cols - 1; x++) {
                                    for (auto y = 1; y < num_rows - 1; y++) {
                                        blocks[x + y * num_cols].setAir();
                                    }
                                }
                                break;

                            case SDLK_0:
                                // set all active water blocks to air
                                for (auto x = 1; x < num_cols - 1; x++) {
                                    for (auto y = 1; y < num_rows - 1; y++) {
                                        auto& block = blocks[x + y * num_cols];

                                        if (block.type == material::WATER) {
                                            block.setAir();
                                        }
                                    }
                                }
                                break; 

                            case SDLK_1:
                            {
                                // print out the data stored in the selected block
                                auto const & block = blocks[mouse_grid_x + mouse_grid_y * num_cols];
                                std::cout << "\n---------\n";
                                std::cout << "material: " << (unsigned)block.type << "\nmass: " << +block.mass << "\nmass_next: " << +block.mass_next << "\n";
                                break;
                            } 
                        }
                        break; // SDL_KEYDOWN
                }
            }
           
            // act based on which mouse button is clicked 
            // we cannot do this in the event poll loop as we want a continuos 
            // flow of water or ground blocks 
            {            
                auto& current_block {blocks[mouse_grid_x + mouse_grid_y * num_cols]};

                if (left_down) {
                    switch (current_block.type) {
                        case material::WATER: 
                            [[fallthrough]]; 
                        case material::AIR: 
                            current_block.setGround();
                            break;
                    }
                } 

                if (right_down) {
                    current_block.type = material::WATER;
                    current_block.mass = constants::max_mass; 
                    current_block.mass_next = constants::max_mass;
                }
            }
            
            // update
            {
               
                for (int y = 0; y < num_rows; y++) {
                   for (int x = 0; x < num_cols; x++) {
                       
                        auto& current_block = blocks[x + y * num_cols];

                        auto remaining = current_block.mass;

                        if (remaining < constants::min_mass) continue;
                        if (current_block.type != material::WATER) continue;         
                        
                        
                        // downwards flow
                        auto& down_block = blocks[x + (y + 1) * num_cols];
                        
                        if (down_block.type != material::GROUND) {
                            auto room = constants::max_mass - down_block.mass;
                            auto flow = std::min(room, remaining);
                            
                            // if flow is negative constrain to 0
                            flow = std::max(flow, 0.0f);

                            if (flow > std::min(constants::max_mass, current_block.mass))
        						flow = std::min(constants::max_mass, current_block.mass);

                            current_block.mass_next -= flow;
                            down_block.mass_next += flow ;
                            remaining -= flow;
                        }   


                        // if no water left to work with, skip to next loop
                        if (remaining < constants::min_mass) continue; 
                        

                        // left flow
                        auto left_index = (x - 1) + y * num_cols;
                        
                        if (blocks[left_index].type != material::GROUND) {
                            
                            auto room = constants::max_mass - blocks[left_index].mass;
                            auto flow = std::min(room, remaining) / 3.0f;
                            
                            flow = std::max(flow, 0.0f);

                            if (flow > std::min(constants::max_mass, current_block.mass))
        						flow = std::min(constants::max_mass, current_block.mass);

                            current_block.mass_next -= flow;
                            blocks[left_index].mass_next += flow;
                            remaining -= flow;
                        }
                        
                        
                        // if no water left to work with, skip to next loop
                        if (remaining < constants::min_mass) continue; 

                        
                        // right flow
                        auto right_index = (x + 1) + y * num_cols;
                      
                        if (blocks[right_index].type != material::GROUND) {
                            auto room = constants::max_mass - blocks[right_index].mass;
                            auto flow = std::min(room, remaining) / 2.0f;
                            
                            flow = std::max(flow, 0.0f);

                            if (flow > std::min(constants::max_mass, current_block.mass))
        						flow = std::min(constants::max_mass, current_block.mass);


                            current_block.mass_next -= flow;
                            blocks[right_index].mass_next += flow;
                            remaining -= flow;
                        }
                   }
                }

                for (block & b : blocks) {
                    if (b.type == material::GROUND) continue;
                    b.mass = b.mass_next;

                    if (b.mass < constants::min_mass) {
                        b.setAir();
                    } else {
                        b.type = material::WATER;
                        
                        // clamp the mass 
                        if (b.mass > constants::max_mass) {
                            b.mass = constants::max_mass;
                        }
                    }
                }
            }

            // render 
            {
                SDL_SetRenderDrawColor(sdl.m_renderer,constants::air.r, constants::air.g, constants::air.b, 255); 
                sdl.clear_back_buffer();

                for (auto y = 0; y < num_rows; y++) {
                    for (auto x = 0; x < num_cols; x++) {
                        
                        auto const & block_to_render {blocks[x + y * num_cols]};
                        
                        SDL_Rect r;
                        r.x =   x * cell_size;
                        r.y =  (y * cell_size) + cell_size; 
                        r.w =  cell_size;
                        r.h = -cell_size;

                       
                        switch (block_to_render.type) {

                            case material::AIR:
                                break;

                            case material::GROUND:
                                SDL_SetRenderDrawColor(sdl.m_renderer, 24, 44, 76,255);
                                SDL_RenderFillRect(sdl.m_renderer, &r); 
                                break;

                            case material::WATER:
                            {
                                auto above_mass {blocks[x + (y - 1) * num_cols].mass};

                                colour mass_colour(lerp_colour(constants::min_mass_colour, constants::max_mass_colour, block_to_render.mass / constants::max_mass));

                                if (!(above_mass != 0 && block_to_render.mass != 0)) { // if the block is falling
                                    r.h = -cell_size * (block_to_render.mass / constants::max_mass); // scale the box depending on the amount of water in the cell
                                }
                                
                                SDL_SetRenderDrawColor(sdl.m_renderer, mass_colour.r, mass_colour.g, mass_colour.b, 255);
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

                SDL_SetRenderDrawColor(sdl.m_renderer, constants::selection.r, constants::selection.g, constants::selection.b, 255);
                SDL_RenderDrawRect(sdl.m_renderer, &mouse_position);

                sdl.back_to_front();
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
