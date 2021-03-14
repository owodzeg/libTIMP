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

    int pal_colors = 16;

    uint32_t pal_data,px_data;

    TIMP.seekg(0x20);
    TIMP.read(reinterpret_cast<char*>(&pal_data), sizeof(uint32_t));

    TIMP.seekg(0x24);
    TIMP.read(reinterpret_cast<char*>(&px_data), sizeof(uint32_t));

    ///pal_data == 0 when palette is not defined
    if(pal_data != 0x0)
    {
        pal_colors = (px_data - 0x30) / 0x4;
        cout << pal_colors << " color TIMP" << endl;
    }
    else
    {
        pal_colors = 0;
    }

    /*int w = floor(width / 16);
    int h = floor(height / 16);

    if(w%16 != 0)
    {
        float l = floor(log2(w*16));
        float nw = pow(2,l+1);
        width = nw;

        cout << w << " " << l << " " << nw << endl;
    }

    if(h%16 != 0)
    {
        float l = floor(log2(h*16));
        float nh = pow(2,l+1);
        height = nh;

        cout << h << " " << l << " " << nh << endl;
    }*/

    //system("pause");

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

    if(pal_data != 0x0)
    {
        for(int i=0; i<px_length; i++)
        {
            int pixel;

            TIMP.seekg(px_data+i);
            TIMP.read(reinterpret_cast<char*>(&pixel), sizeof(uint8_t));

            if(pal_colors == 256)
            {
                pixels.push_back(pixel);
            }
            else if((pal_colors == 16) || (pal_colors == 48))
            {
                pixels.push_back(pixel&0xF);
                pixels.push_back((pixel>>4)&0xF);
            }
        }
    }
    else
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

                }

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

    return image;
}
