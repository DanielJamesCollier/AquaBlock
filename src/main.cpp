#include <iostream>
#include <cstdint>
#include <array>
#include <algorithm>                
#include <cmath>

#include "sdl_module.hpp"

std::uint8_t min_liquid = 10;
std::uint8_t max_liquid = 255;

enum class block_type : std::uint8_t {
    AIR    = 0,
    GROUND = 1,
    WATER  = 2
}; 


template<std::size_t grid_size>
struct blocks_soa {

   blocks_soa() {
       clear();
   }

   void clear() {
      type.fill(block_type::AIR);
      liquid.fill(0);
   }
   
   std::array<block_type, grid_size> type; 
   std::array<std::uint8_t, grid_size> liquid;
};

int main() {
    constexpr int num_cols  = 30; // width 
    constexpr int num_rows  = 10;  // height 
    constexpr int grid_size = num_cols * num_rows;
    constexpr int cell_size = 20;

    try {
        sdl_module sdl("AquaBlock", num_cols * cell_size , num_rows * cell_size);
        
        // data 
        blocks_soa<grid_size> blocks_front;
        blocks_soa<grid_size> blocks_back;
        std::array<std::uint32_t, grid_size> pixels;
        bool running = true; 
        int mouse_x = 0, mouse_y = 0; 
        //..
        

        // fill the bottom and the top with ground blocks
        
        int y_ground = num_rows - 1; 
        int y_roof = 0;
        for(int x = 0; x < num_cols; x++) {
            int roof_index = x + y_roof * num_cols;
            int ground_index = x + y_ground * num_cols;
            blocks_front.type[roof_index] = block_type::GROUND;
            blocks_front.type[ground_index] = block_type::GROUND;

        }
        
        // fill the right and left sides with a ground block 
        int x_left = 0;
        int x_right = num_cols - 1;
        for (int y = 0; y < num_rows; y++) {
            int left_index = x_left + y * num_cols;
            int right_index = x_right + y * num_cols;
            blocks_front.type[left_index] = block_type::GROUND;
            blocks_front.type[right_index] = block_type::GROUND;
        }

        while(running) {
            
            // poll input      
            while (SDL_PollEvent(&sdl.m_event)) {
                switch (sdl.m_event.type) {
                    case SDL_QUIT:
                        running = false; 
                        break;
                    case SDL_MOUSEMOTION:
                        mouse_x = sdl.m_event.motion.x;
                        mouse_y = sdl.m_event.motion.y;
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        
                        int x = std::floor((mouse_x / cell_size) + 0.5f);
                        int y = std::floor((mouse_y / cell_size) + 0.5f); 
                        int index = x + y * num_cols;

                        switch (sdl.m_event.button.button) {
                            case SDL_BUTTON_LEFT: // place ground block
                            {

                                auto& clicked_block_type   = blocks_front.type[index];
                                auto& clicked_block_liquid = blocks_front.liquid[index];

                                switch (clicked_block_type) {
                                    case block_type::AIR:
                                        clicked_block_type = block_type::GROUND;
                                        clicked_block_liquid = 0;
                                        break;

                                    case block_type::GROUND: 
                                        [[fallthrough]]; // ooo fancy

                                    case block_type::WATER: 
                                        clicked_block_type = block_type::AIR;
                                        clicked_block_liquid = 0;
                                        break;
                                } 
                            } break;

                            case SDL_BUTTON_RIGHT: // place water block
                                blocks_front.type[index] = block_type::WATER;
                                blocks_front.liquid[index] = 255;
                                break;
                        }
                        break;
                }
            }

            // render 
            {
                SDL_SetRenderDrawColor(sdl.m_renderer,76, 76, 76, 255); // background colour
                sdl.clear_back_buffer();
                
                for (int y = 0; y < num_rows; y++) {
                    for (int x = 0; x < num_cols; x++) {
                        int index = x + y * num_cols;
                        auto& current_type = blocks_front.type[index]; 

                        SDL_Rect r;
                        r.x = x * cell_size + cell_size;
                        r.y = y * cell_size + cell_size; 
                        r.w = -cell_size;
                        r.h = -cell_size;
                        
                        switch (current_type) {
                            case block_type::AIR: // if air do nothing
                                break;

                            case block_type::GROUND:
                                SDL_SetRenderDrawColor(sdl.m_renderer, 102,50,0,255);
                                SDL_RenderFillRect(sdl.m_renderer, &r); 
                                break;

                            case block_type::WATER:
                                SDL_SetRenderDrawColor(sdl.m_renderer, 0,102,255,255);
                                r.h = - cell_size * (blocks_front.liquid[index] / 255.0f); // scale the box depending on the amount of water in the cell
                                SDL_RenderFillRect(sdl.m_renderer, &r); 
                                break;
                        } 
                    }
                }    

                sdl.back_to_front();
            }

            // update
            {
                blocks_back.clear();

                // copy ground blocks
                for (auto i = 0; i < blocks_front.type.size(); i++) {
                    if (blocks_front.type[i] == block_type::GROUND) {
                        blocks_back.type[i] = block_type::GROUND;
                        // no need to copy liquid as they all are reset in blocks_back.clear();
                    }
                }

                // loop through all blocks
                for (auto y = 0; y < num_rows; y++) {
                    for (auto x = 0; x < num_cols; x++) {
                        int current_index  {x + y * num_cols};

                        auto current_type {blocks_front.type[current_index]};

                        if (current_type != block_type::WATER) continue;
                        
                        auto current_liquid {blocks_front.liquid[current_index]};
                        auto down_index {x + (y + 1) * num_cols};

                        if (y < num_rows - 1 && // check if we are in bounds [ -1 because we need to check the block bellow for its type]
                            blocks_front.type[down_index] != block_type::GROUND) {
                            
                            auto down_type {blocks_front.type[down_index]};
                            auto down_liquid {blocks_front.liquid[down_index]};

                            if (down_liquid < max_liquid) {
                                blocks_back.type[down_index] = block_type::WATER;
                                blocks_back.liquid[down_index] = 255;
                                current_liquid = 0;
                            }
                        }

                        if (current_liquid < min_liquid) continue; 

                        // water is still left over in the current block so add it to the back buffer 
                        blocks_back.type[current_index] = current_type;
                        blocks_back.liquid[current_index] = current_liquid; 
                    }
                }
                blocks_front = blocks_back;
            }
            SDL_Delay(60);
        } 
    } catch (sdl_module_exception const &e) {
        std::cerr << e.what() << std::endl;

    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;

    } catch (...) {
    
    }

    return 0;
}

               /*         
                    // #Rule 2 - flow left 
                        auto  left = current - 1;
                        auto& left_type = blocks
                        auto& left_liquid = blocks.liquid[left];

                        if (x != 0 && left_type != block_type::GROUND) {
                            float flow = (current_liquid - left_liquid) / 4.0f;     

                            flow = std::max(flow, 0.0f);

                           //blocks.liquid_temp[current] -= flow; 
                           //blocks.liquid_temp[left]    += flow; 
                           //current_liquid -= flow;
                           //left_type = block_type::WATER;

                        }
                        
                        // check if still liquid in cell
                        if (current_liquid < min_liquid) {
                            current_liquid = 0;
                            continue;
                        }

                        // #Rule 2 - flow right 
                        auto  right = current + 1;
                        auto& right_type = blocks.type_front[right];
                        auto& right_liquid = blocks.liquid[right];

                        if (x != num_rows -1 && right_type != block_type::GROUND) {
                            float flow = (current_liquid - right_liquid) / 3.0f;     

                            flow = std::max(flow, 0.0f);

                            //blocks.liquid_temp[current] -= flow; 
                            //blocks.liquid_temp[right]   += flow; 
                            //current_liquid -= flow;
                            //right_type = block_type::WATER;
                        }
                        
                        // check if still liquid in cell
                        if (current_liquid < min_liquid) {
                            current_liquid = 0;
                            continue;
                        }

                        */
