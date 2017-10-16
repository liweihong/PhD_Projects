#include "VCProject.h"
#define thres 0.1
#define PI 3.14159
int log_trans(BYTE** image, const int centroid_x, const int centroid_y, float*
histogram, const int w, const int h);

void scale_translate(BYTE **ppt_image, BYTE **mimio_image, Bitmap *ppt_bitmap,
Bitmap *mimio_bitmap, Bitmap *&newbitmap, double *params)
{
        int i,j;
        int ppt_H = ppt_bitmap->GetHeight();
        int ppt_W = ppt_bitmap->GetWidth();
        int mimio_H = mimio_bitmap->GetHeight();
        int mimio_W = mimio_bitmap->GetWidth();

        //Get the centroid of these 2 images;
        //*1 stands for ppt, *2 stands for mimio;
        int index1 = 0, index1_x = 0, index1_y = 0;
        int index2 = 0, index2_x = 0, index2_y = 0;

        for (i = 0; i < ppt_H ; i++)
                for (j = 0; j < ppt_W; j++)
                {
                        if (ppt_image[i][j] == BLACK)
                        {
                                index1_x += j;
                                index1_y += i;
                                index1 ++;
                        }
                }

        for (i = 0; i < mimio_H; i++)
                for (j = 0; j < mimio_W; j++)
                {
                        if (mimio_image[i][j] == BLACK)
                        {
                                index2_x += j;
                                index2_y += i;
                                index2 ++;
                        }
                }

        int centroid1_x = int(((float)index1_x)/index1);
        int centroid1_y = int(((float)index1_y)/index1);
        int centroid2_x = int(((float)index2_x)/index2);
        int centroid2_y = int(((float)index2_y)/index2);

//        centroid1_y = 209;

        cout<<"centroid1_x"<<centroid1_x<<endl;
        cout<<"centroid1_y"<<centroid1_y<<endl;
        cout<<"centroid2_x"<<centroid2_x<<endl;
        cout<<"centroid2_y"<<centroid2_y<<endl;

        
        float histogram_video[360], histogram_mimio[360];
        for (i=0; i<360; i++)
        {
                histogram_video[i] = 0.0;
                histogram_mimio[i] = 0.0;
        }
        
        int c1=log_trans(ppt_image, centroid1_x, centroid1_y, histogram_video, ppt_W, ppt_H);
        int c2=log_trans(mimio_image, centroid2_x, centroid2_y, histogram_mimio, mimio_W,
mimio_H);
        float sum_video=0.0, sum_mimio=0.0;


        for(i=0; i<360; i++)
        {         
                sum_video += histogram_video[i];
                sum_mimio += histogram_mimio[i];
        }
        float scale1 = sum_video/sum_mimio;
        
        float scale = 1 / scale1;
        cout<<"sum_video"<<sum_video<<endl;
        cout<<"sum_mimio"<<sum_mimio<<endl;
        cout<<"scale1="<<scale1<<"scale="<<scale<<endl;
        
        BYTE **mimio_orig;
        set_image(mimio_orig,mimio_bitmap);

        cout<<"ppt centroid1_x"<<centroid1_x<<"centroid1_y"<<centroid1_y<<endl;
        cout<<"mimio centroid2_x"<<centroid2_x<<"centroid2_y"<<centroid2_y<<endl;

        int trans_x = (int)floor(centroid1_x - centroid2_x * scale1);
        int trans_y = (int)floor(centroid1_y - centroid2_y * scale1);

        int scaled_H = (int)(mimio_H * scale1);
        int scaled_W = (int)(mimio_W * scale1);

        newbitmap = new Bitmap(scaled_W,scaled_H);

        // Bilinear interpolation
        double xx, yy;
        int fx, fy, v3;
        double v1,v2,v3_1,r1,r2;
         for (i=0; i < scaled_H; i++)
                for (j=0; j < scaled_W; j++)
                {
                        newbitmap->SetPixel(j,i,Color(255,255,255));
                        xx = j * scale;
                        yy = i * scale;
                        fx = (int)floor(xx);
                        fy = (int)floor(yy);
                        r1 = xx - fx; r2 = yy - fy;
                        if (i + trans_y > 0 && i + trans_y < scaled_H && j + trans_x > 0 && j + trans_x <
scaled_W
                                && fx >= 0 && fx+1 < mimio_W && fy >= 0 && fy+1 < mimio_H)
                        {
                                v1 = mimio_orig[(int)ceil(yy)][(int)floor(xx)] +
                                                        (mimio_orig[(int)ceil(yy)][(int)ceil(xx)] - 
                                                        mimio_orig[(int)ceil(yy)][(int)floor(xx)]) * r1;
                                v2 = mimio_orig[(int)floor(yy)][(int)floor(xx)] +
                                                        (mimio_orig[(int)floor(yy)][(int)ceil(xx)] -
                                                        mimio_orig[(int)floor(yy)][(int)floor(xx)]) * r1;
                                v3_1 = (v2 + (v1 - v2) * r2);
                                if (v3_1 != 0 ) v3 = BLACK; else v3 = WHITE;
                                newbitmap->SetPixel(j+trans_x,i+trans_y,Color((1 - v3)*255, (1 - v3)*255,(1 -
v3)*255));
                        }
                }


        /* NN interpolation
        int xx, yy;
         for (i=0; i < scaled_H; i++)
                for (j=0; j < scaled_W; j++)
                {
                        newbitmap->SetPixel(j,i,Color(255,255,255));
                        xx = (int)(j * scale);
                        yy = (int)(i * scale);
                        if (i + trans_y > 0 && i + trans_y < scaled_H && j + trans_x > 0 && j + trans_x <
scaled_W
                                && xx > 0 && xx < mimio_W && yy > 0 && yy < mimio_H)
                                newbitmap->SetPixel(j+trans_x,i+trans_y,Color((1 - mimio_orig[yy][xx])*255,
(1-mimio_orig[yy][xx])*255,(1-mimio_orig[yy][xx])*255));
                }
        */

        delete [] mimio_orig[0];
        delete [] mimio_orig;
        
        cout<<"scale="<<scale1<<endl;
        cout<<"trans_x"<<trans_x<<endl;
        cout<<"trans_y"<<trans_y<<endl;

        params[0] = scale1;
        params[1] = trans_x;
        params[2] = trans_y;
}

int log_trans(BYTE** image, const int centroid_x, const int centroid_y, float*
histogram, const int w, const int h)
{
        //init angle
        int angle=0;
        int y, x, i, j;
        float yy=0.0, xx=0.0;
        float dx, dy, cosa, sina;
        float intensity=0.0;
        float length_left=0.0, length_right=0.0;
        float x_first, y_first, x_last, y_last;
        int c=0;

        for(angle=0; angle<180; angle++ )
        {
                dx = cosa = cos (PI*angle/180);
                dy = sina = sin (PI*angle/180);
                if (angle == 0)
                {
                        for(x=0; x<centroid_x && image[centroid_y][x]!=1; x++)
                        {}
                        histogram[180] = abs(x-centroid_x);
                        for(x=w-1; x>centroid_x && image[centroid_y][x]!=1; x--)
                        {}
                        histogram[0] = abs(x-centroid_x);
                }
                else
                {        
                        if (angle == 90)
                        {
                                for(y=0; y<centroid_y && image[y][centroid_x]!=1; y++)
                                {}
                                histogram[270] = abs(y-centroid_y);
                                for(y=h-1; y>centroid_y && image[y][centroid_x]!=1; y--)
                                {}
                                histogram[90] = abs(y-centroid_y);
                        }
                        else
                        {        
                                if (angle < 90)
                                {

                                        length_left  = floor(MIN(centroid_y/sina, centroid_x/cosa));
                                        length_right = floor(MIN((h-centroid_y)/sina,(w-centroid_x)/cosa));
                                        x_first = centroid_x - length_left*cosa;
                                        y_first = centroid_y - length_left*sina;
                                        x_last = centroid_x + length_right*cosa;
                                        y_last = centroid_y + length_right*sina;
                                }

                                else
                                {

                                        length_right = floor(MIN(centroid_y/sina, (w-centroid_x)/(-cosa)));
                                        length_left  = floor(MIN(centroid_x/(-cosa), (h-centroid_y)/sina));
                                        x_first = centroid_x - length_right*cosa;
                                        y_first = centroid_y - length_right*sina;
                                        x_last = centroid_x + length_left*cosa;
                                        y_last = centroid_y + length_left*sina;
                                        float temp = length_left;
                                        length_left = length_right;
                                        length_right = temp;


                                }
        
if(angle!=0 && angle!=90)
{
                                xx = x_first;
                                yy = y_first;
                                for (int m=0; m<length_left; m++)
                                {
                                        float x_dis_FLOOR = xx - floor(xx);  //float x_dis_FLOOR;
                                        float y_dis_FLOOR = yy - floor(yy);  //float y_dis_FLOOR;                                                
                                        int x_f = floor (xx);
                                        int y_f = floor (yy);
                                        if (x_f>=w-1 || y_f>=h-1)
                                        {                                        
                                                xx += dx;
                                                yy += dy;
                                                continue;
                                        }
                                        //ulv: the intensity value of up-left neighbor of current pixel; 
                                        float ulv = image[y_f][x_f];
                                        float urv = image[y_f][x_f+1];
                                        float llv = image[y_f+1][x_f];
                                        float lrv = image[y_f+1][x_f+1];

                                        intensity = (float)(1-x_dis_FLOOR)*(llv*y_dis_FLOOR + ulv*(1-y_dis_FLOOR))
                                                                + x_dis_FLOOR*(lrv*y_dis_FLOOR + urv*(1-y_dis_FLOOR));
                                        if (intensity>thres)
                                        {
                                                histogram[180-angle] = sqrt(
(yy-centroid_y)*(yy-centroid_y)+(xx-centroid_x)*(xx-centroid_x) );
                                                c++;;
                                                break;
                                        }

                                        xx += dx;
                                        yy += dy;
                                }//for

                                xx = x_last;
                                yy = y_last;
                                for (m=0; m<length_right; m++)
                                {
                                        float x_dis_FLOOR = xx - floor(xx);  //float x_dis_FLOOR;
                                        float y_dis_FLOOR = yy - floor(yy);  //float y_dis_FLOOR;                                                
                                        int x_f = floor (xx);
                                        int y_f = floor (yy);
                                        if (x_f>=w-1 || y_f>=h-1)
                                        {                                        
                                                xx -= dx;
                                                yy -= dy;
                                                continue;
                                        }
                                        float ulv = image[y_f][x_f];
                                        float urv = image[y_f][x_f+1];
                                        float llv = image[y_f+1][x_f];
                                        float lrv = image[y_f+1][x_f+1];

                                        intensity = (float)(1-x_dis_FLOOR)*(llv*y_dis_FLOOR + ulv*(1-y_dis_FLOOR))
                                                                + x_dis_FLOOR*(lrv*y_dis_FLOOR + urv*(1-y_dis_FLOOR));
                                        if (intensity>thres)
                                        {
                                                histogram[360-angle] = sqrt(
(yy-centroid_y)*(yy-centroid_y)+(xx-centroid_x)*(xx-centroid_x) );
                                                c++;
                                                break;

                                        }

                                        xx -= dx;
                                        yy -= dy;
                                }//for
                        }//else
                }//else
        }
}

        cout<<" c= "<<c<<endl;
        return c+4;
}
