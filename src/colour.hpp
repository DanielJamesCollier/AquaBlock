#ifndef COLOUR_HPP
#define COLOUR_HPP

struct colour {

    constexpr colour(int rgb) 
    : r{rgb}
    , g{rgb}
    , b{rgb}{}

    constexpr colour(int r, int g, int b)
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

    auto lerp = [](auto a, auto b, float t) {
        return (1.0f - t) * a + t * b;
    };
    
    colour.r = lerp(lhs.r, rhs.r, amount);
    colour.g = lerp(lhs.g, rhs.g, amount);
    colour.b = lerp(lhs.b, rhs.b, amount);

    return colour;
}
#endif // colour_hpp
