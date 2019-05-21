#ifndef LIBTIMP_HPP
#define LIBTIMP_HPP

#include <iostream>
#include <SFML/Graphics.hpp>
#include <fstream>

class libTIMP
{
    public:
    uint16_t width, height;
    int chunk_w,chunk_h;

    std::vector<int> pixels;
    std::vector<sf::Color> pal_color;

    libTIMP();
    void load(std::string file);
    sf::Image convertToImage();
};

#endif // LIBTIMP_HPP
