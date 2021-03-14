#include "libTIMP.hpp"

libTIMP::libTIMP()
{

}

void libTIMP::load(std::string file)
{
    std::ifstream TIMP(file, std::ios::binary);

    ///Read width and height
    TIMP.seekg(0x12);
    TIMP.read(reinterpret_cast<char*>(&width), sizeof(uint16_t));

    TIMP.seekg(0x14);
    TIMP.read(reinterpret_cast<char*>(&height), sizeof(uint16_t));

    int pal_colors = 16;

    uint32_t pal_data,px_data;

    ///Read offsets
    TIMP.seekg(0x20);
    TIMP.read(reinterpret_cast<char*>(&pal_data), sizeof(uint32_t));

    TIMP.seekg(0x24);
    TIMP.read(reinterpret_cast<char*>(&px_data), sizeof(uint32_t));

    ///Pal_data == 0 when palette is not defined
    if(pal_data != 0x0)
    {
        pal_colors = (px_data - 0x30) / 0x4;
    }
    else
    {
        pal_colors = 0;
    }

    ///Apply swizzling
    if(pal_colors == 16)
    {
        chunk_w = 32;
        chunk_h = 8;
    }
    else if(pal_colors == 256)
    {
        chunk_w = 16;
        chunk_h = 8;
    }
    else if(pal_colors == 48)
    {
        chunk_w = 32;
        chunk_h = 8;
    }
    else if(pal_colors == 0)
    {
        chunk_w = 4;
        chunk_h = 8;
    }

    ///Read palette
    for(int i=0; i<pal_colors; i++)
    {
        uint8_t r,g,b,a;

        TIMP.seekg(pal_data + i*4);
        TIMP.read(reinterpret_cast<char*>(&r), sizeof(uint8_t));

        TIMP.seekg(pal_data + i*4 + 1);
        TIMP.read(reinterpret_cast<char*>(&g), sizeof(uint8_t));

        TIMP.seekg(pal_data + i*4 + 2);
        TIMP.read(reinterpret_cast<char*>(&b), sizeof(uint8_t));

        TIMP.seekg(pal_data + i*4 + 3);
        TIMP.read(reinterpret_cast<char*>(&a), sizeof(uint8_t));

        pal_color.push_back(sf::Color(r,g,b,a));
    }

    ///Amount of pixels
    int px_length = width*height;

    ///If palette exists, read pixels according to palette
    if(pal_data != 0x0)
    {
        for(int i=0; i<px_length; i++)
        {
            int pixel;

            TIMP.seekg(px_data+i);
            TIMP.read(reinterpret_cast<char*>(&pixel), sizeof(uint8_t));

            if(pal_colors == 256) ///8 bits per pixel
            {
                pixels.push_back(pixel);
            }
            else if((pal_colors == 16) || (pal_colors == 48)) ///4 bits per pixel
            {
                pixels.push_back(pixel&0xF);
                pixels.push_back((pixel>>4)&0xF);
            }
        }
    }
    else ///If palette doesnt exist, read colors from pixels
    {
        for(int i=0; i<px_length; i++)
        {
            uint32_t pixel;

            uint8_t r,g,b,a;

            TIMP.seekg(px_data + i*4);
            TIMP.read(reinterpret_cast<char*>(&r), sizeof(uint8_t));

            TIMP.seekg(px_data + i*4 + 1);
            TIMP.read(reinterpret_cast<char*>(&g), sizeof(uint8_t));

            TIMP.seekg(px_data + i*4 + 2);
            TIMP.read(reinterpret_cast<char*>(&b), sizeof(uint8_t));

            TIMP.seekg(px_data + i*4 + 3);
            TIMP.read(reinterpret_cast<char*>(&a), sizeof(uint8_t));

            pal_color.push_back(sf::Color(r,g,b,a));
            pixels.push_back(pal_color.size()-1);
        }
    }

    TIMP.close();
}

sf::Image libTIMP::convertToImage()
{
    ///Create empty image
    sf::Image image;
    image.create(width,height,sf::Color(0,0,0,0));

    int x=0,y=0;
    unsigned int p=0;

    ///Assign pixels with swizzling
    while(p < pixels.size())
    {
        for(int ch=0; ch<chunk_h; ch++)
        {
            for(int cw=0; cw<chunk_w; cw++)
            {
                if(x+cw < width)
                if(y+ch < height)
                {
                    image.setPixel(x+cw,y+ch,pal_color[pixels[p]]);
                }

                ///Pixels are incremented nonetheless so swizzling can work on textures with sizes that are not a power of 2
                p++;
            }
        }

        x+=chunk_w;

        if(x >= width)
        {
            x=0;
            y+=chunk_h;
        }

        if(y >= height)
        {
            break;
        }
    }

    ///Return finished sf::Image
    return image;
}
