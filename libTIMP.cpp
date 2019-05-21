#include "libTIMP.hpp"

using namespace std;

libTIMP::libTIMP()
{

}

void libTIMP::load(std::string file)
{
    ifstream TIMP(file, std::ios::binary);

    cout << "Reading " << file << endl;

    TIMP.seekg(0x12);
    TIMP.read(reinterpret_cast<char*>(&width), sizeof(uint16_t));

    TIMP.seekg(0x14);
    TIMP.read(reinterpret_cast<char*>(&height), sizeof(uint16_t));

    uint8_t pal_type;
    int pal_colors = 16;

    TIMP.seekg(0x19);
    TIMP.read(reinterpret_cast<char*>(&pal_type), sizeof(uint8_t));

    if(pal_type == 4)
    {
        cout << "16 color TIMP" << endl;
        pal_colors = 16;
        chunk_w = 32;
        chunk_h = 8;
    }
    else if(pal_type == 5)
    {
        cout << "256 color TIMP" << endl;
        pal_colors = 256;
        chunk_w = 16;
        chunk_h = 8;
    }

    uint32_t pal_data,px_data;

    TIMP.seekg(0x20);
    TIMP.read(reinterpret_cast<char*>(&pal_data), sizeof(uint32_t));

    TIMP.seekg(0x24);
    TIMP.read(reinterpret_cast<char*>(&px_data), sizeof(uint32_t));

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

    int px_length = width*height;

    for(int i=0; i<px_length; i++)
    {
        int pixel;

        TIMP.seekg(px_data+i);
        TIMP.read(reinterpret_cast<char*>(&pixel), sizeof(uint8_t));

        if(pal_type == 5)
        {
            pixels.push_back(pixel);
        }
        else if(pal_type == 4)
        {
            pixels.push_back(pixel&0xF);
            pixels.push_back((pixel>>4)&0xF);
        }
    }

    TIMP.close();
}

sf::Image libTIMP::convertToImage()
{
    sf::Image image;
    image.create(width,height,sf::Color(0,0,0,0));

    int x=0,y=0;
    unsigned int p=0;

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
                    p++;
                }
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

    return image;
}
