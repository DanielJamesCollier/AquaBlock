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


template<std::size_t rows, std::size_t columns>
struct blocks_soa {

   blocks_soa() 
   :    type{block_type::AIR}
   ,    liquid{0}{
       // empty
   }

   void clear() {
      type.fill(block_type::AIR);
      liquid.fill(0);
   }
   
   std::array<block_type, rows * columns> type; 
   std::array<std::uint8_t, rows * columns> liquid;
};

int main() {
    constexpr int num_cols  = 20; // height 
    constexpr int num_rows  = 30; // width
    constexpr int cell_size = 30;

    try {
        sdl_module sdl("AquaBlock", num_rows * cell_size , num_cols * cell_size);
        
        // data 
        blocks_soa<num_rows, num_cols> blocks_front;
        blocks_soa<num_rows, num_cols> blocks_back;
        std::array<std::uint32_t, num_rows * num_cols> pixels;
        bool running = true; 
        int mouse_x = 0, mouse_y = 0; 
        //..
        
        // fill bottom line with ground
        int y_ground = num_cols - 1; 
        for(int x = 0; x < num_rows; x++) {
            int index = x + y_ground * num_rows;
            blocks_front.type[index] = block_type::GROUND;

        }

        // fill left hand side with gound
        int x_left = 0;
        for (int y = 0; y < num_cols; y++) {
            int index = x_left + y * num_rows;
            blocks_front.type[index] = block_type::GROUND;
        }
        
        // fill right hand side with gound
        int x_right = num_rows - 1;
        for (int y = 0; y < num_cols; y++) {
            int index = x_right + y * num_rows;
            blocks_front.type[index] = block_type::GROUND;
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
                        
                        int x = std::floor(mouse_x / num_rows);
                        int y = std::floor(mouse_y / num_rows);
                        int index = x + y * num_rows;
                        
                        switch (sdl.m_event.button.button) {
                            case SDL_BUTTON_LEFT: // place ground block
                                blocks_front.type[index] = block_type::GROUND;
                                blocks_front.liquid[index] = 0;
                                break; 
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
                
                for (int y = 0; y < num_cols; y++) {
                    for (int x = 0; x < num_rows; x++) {
                        int index = x + y * num_rows;
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
                
                // copy ground blocks from front to back
                for (std::size_t i = 0; i < blocks_front.type.size(); i++) {
                    if (blocks_front.type[i] == block_type::GROUND) {
                        blocks_back.type[i] = block_type::GROUND;
                    }
                }
                
                for (int y = 0; y < num_cols; y++) {
                    for (int x = 0; x < num_rows; x++) {

                        int current = x + y * num_rows;
                        int down    = x + (y + 1) * num_rows;

                        const auto& current_type = blocks_front.type[current];
                        std::uint8_t current_liquid = blocks_front.liquid[current];
                        
                        if (current_type != block_type::WATER) continue;

                        if (y < num_cols && // check if in bounds
                            blocks_front.type[down] != block_type::GROUND) { // make sure block bellow isnt ground 
                            
                            const auto& down_type    = blocks_front.type[down];
                            std::uint8_t down_liquid = blocks_front.liquid[down];

                            if (down_liquid == 0) {
                                
                                blocks_back.liquid[down] = 255;
                                blocks_back.type[down]   = block_type::WATER;            
                                current_liquid = 0;    
                            }


                        }
                        
                        // check we have some water to work with if not write to buffer and skip
                        if (current_liquid < min_liquid) {
                            current_liquid = 0;
                            blocks_back.liquid[current] = current_liquid;
                            blocks_back.type[current]   = block_type::AIR; 
                            continue;
                        }
                        
                        // we still have water left in this blocks so write it to the buffer 
                        blocks_back.liquid[current] = current_liquid;
                        blocks_back.type[current]   = block_type::WATER;
                    }
                }
                
                // copy back to front
                blocks_front = blocks_back;
            } 
        
        
            SDL_Delay(200);
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
                        auto& left_type = blocks.type_front[left];
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
