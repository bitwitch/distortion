#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define PI 3.14159265358979323846

typedef unsigned char u8;
typedef char i8;

class Demo : public olc::PixelGameEngine
{
public:
    Demo() { sAppName = "Distortion"; }

    int HEIGHT, WIDTH;
    int half_height, half_width;

    // buffer indices
    int x1, y1, x2, y2;

    i8 *dispX, *dispY;

    olc::Sprite *image;

    float current_time;

public:
    /*
     * calculate a distorion function for X and Y in 5.3 fixed point
     */
    void precalculate()
    {
        int i, j, dst;
        dst = 0;
        for (j=0; j<HEIGHT*2; j++)
        {
            for (i=0; i<WIDTH*2; i++)
            {
                float x = (float)i;
                float y = (float)j;
             // notice the values contained in the buffers are signed
             // i.e. can be both positive and negative
                dispX[dst] = (signed char)(8*(2*(sin(x/20) + sin(x*y/2000)
                           + sin((x+y)/100) + sin((y-x)/70) + sin((x+4*y)/70)
                           + 2*sin(hypot(256-x,(150-y/8))/40) )));
             // also notice we multiply by 8 to get 5.3 fixed point distortion
             // coefficients for our bilinear filtering
                dispY[dst] = (signed char)(8*((cos(x/31) + cos(x*y/1783) +
                           + 2*cos((x+y)/137) + cos((y-x)/55) + 2*cos((x+8*y)/57)
                           + cos(hypot(384-x,(274-y/9))/51) )));
                dst++;
            }
        }
    }

    /*
     *   copy an image to the screen with added distortion.
     *   no bilinear filtering.
     */
    void Distort(olc::Sprite *img, int x1, int y1, int x2, int y2)
    {
    // setup the offsets in the buffers
         int dst = 0,
             src1 = y1*WIDTH*2+x1,
             src2 = y2*WIDTH*2+x2;
         int dX, dY;
         u8 c;
         for (int j=0; j<HEIGHT; j++) {
             for (int i=0; i<WIDTH; i++) {
                 // get distorted coordinates, use the integer part of the distortion
                 // buffers and truncate to closest texel
                 dY = j+(dispY[src1]>>3);
                 dX = i+(dispX[src2]>>3);
                 // draw texel or black
                 if (dY>=0 && dY<HEIGHT-1 && dX>=0 && dX<WIDTH-1)
                     Draw(i, j, img->GetPixel(dX, dY));
                 else 
                     Draw(i, j, olc::BLACK); 
                 
                  // next pixel
                 dst++; src1++; src2++;
             }
              // next line
             src1 += WIDTH;
             src2 += WIDTH;
         }
    }

    bool OnUserCreate() override 
    {
        HEIGHT = ScreenHeight();
        WIDTH = ScreenWidth(); 
        half_height = HEIGHT / 2;
        half_width = WIDTH / 2;

        current_time = 0;

        dispX = new i8[WIDTH*2+HEIGHT*2];
        dispY = new i8[WIDTH*2+HEIGHT*2];

        image = new olc::Sprite("xen.png");

        return true;    
    }

    bool OnUserUpdate(float fElapsedTime) override 
    {
        current_time += fElapsedTime * 1000;

        // move distortion buffer
        x1 = half_width + (int)((half_width-1) * cos( (double)current_time/205 ));
        x2 = half_width + (int)((half_width-1) * sin( (double)-current_time/197 ));
        y1 = half_height + (int)((half_height-1) * sin( (double)current_time/231 ));
        y2 = half_height + (int)((half_height-1) * cos( (double)-current_time/224 ));

        // draw the effect
        //if ((current_time & 511.0f) < 256) 
            Distort(image, x1, y1, x2, y2);

        //DrawSprite(0, 0, image);

        return true;    
    }

    bool OnUserDestroy() override 
    {
        delete [] dispX;
        delete [] dispY;
        delete image;
        return true;    
    }
};


int main () {

    Demo demo;
    if (demo.Construct(800, 600, 1, 1)) {
        demo.Start();
    }
    return 0;
}


