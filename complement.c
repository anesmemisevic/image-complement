//
//  main.c
//  complement
//
//  Created by Anes Memišević on 29. 5. 2021..
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#define allignment 1


/* In order to align the data in memory, one or more empty bytes (addresses) are inserted
 but in our .bmp structure, the memory has its own specific style of allignment, in bitmapfileheader:
 there would exist some empty memory in-between components since the memory goes like (in bytes):
 2,4,2,2,4. Because of this, we must override the default action and edit it according to the .bmp structure
 Therefore, edit it so that no padding in between component would be inserted. In that way we preserve the 14 bytes which is usually the size of BITMAPFILEHEADER.
 
 References: https://docs.microsoft.com/en-us/cpp/preprocessor/pack?view=msvc-160
 & https://stackoverflow.com/questions/3318410/pragma-pack-effect
 
 "To pack a class is to place its members directly after each other in memory. -- WE NEED THIS!
 It can mean that some or all members can be aligned on a boundary smaller than the default alignment of the target architecture."
 */

#pragma pack(allignment)
typedef struct {
    /*
     The .bmp file format info, references and file header/info header components' names taken from:
     https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html
     & http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
     & https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
     */
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
}bitmapfileheader_t;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
}bitmapinfoheader_t;

typedef struct {
    uint8_t rgbBlue; // specifies the blue part of the color
    uint8_t rgbGreen; // specifies the green part of the color
    uint8_t rgbRed; // specifies the red part of the color
    //    uint8_t rgbReserved; // comment out, handled, always set to zero
}RGBQUAD_t;

int main(int argc, char *argv[]) {
    //open my teduid card image
    FILE *image_input = fopen("marbles.bmp", "rb");
    if (!image_input){
        printf("Image not found for input\n");
        return -1;
    }
    //image which will be output as complement
    FILE *image_output = fopen("img_complement.bmp", "wb"); // create a new img.
    
    bitmapfileheader_t bmfh;
    bitmapinfoheader_t bmih;
    
    // read to structs
    fread(&bmfh, sizeof(bitmapfileheader_t), 1, image_input);
    fread(&bmih, sizeof(bitmapinfoheader_t), 1, image_input);
    // copy the parts which will remain the same as in the original picture: header, other info etc.
    fwrite(&bmfh, sizeof(bitmapfileheader_t), 1, image_output);
    fwrite(&bmih, sizeof(bitmapinfoheader_t), 1, image_output);
    
    //care for padding as explained before
    int padding = 0;
    int padding_needed = 0;
    if (bmih.biWidth % 4 != 0) {
        padding = 4 - bmih.biWidth % 4;
        padding_needed = 1;
    }
    //    printf("%d",padding);
    
    int height = bmih.biHeight;
    int width = bmih.biWidth;
    int i = 0;
    for (i = height; i >= 0; i--) { // according to the documentation:
        /* Scan lines are stored bottom to top instead of top to bottom
         RGB values are stored backwards i.e. BGR
         */
        for (int j = 0; j < width; j++) {
            /*
             Edit every pixel value accordingly and copy it to the new output image
             */
            RGBQUAD_t COLORS;
            fread(&COLORS, sizeof(RGBQUAD_t), 1, image_input);

            //for black and white conversion:
                        float avg = (COLORS.rgbBlue+COLORS.rgbGreen+COLORS.rgbRed)/3.0;
                        avg = round(avg); // include math.h for round
            
                        avg = 255 - avg; // for b&w complement
                        COLORS.rgbBlue = avg;
                        COLORS.rgbRed = avg;
                        COLORS.rgbGreen = avg;
            
            fwrite(&COLORS, sizeof(RGBQUAD_t), 1, image_output);
        }
    }
    
    if(padding_needed){
        /* According to the documentation:
         Each scan line is zero padded to the nearest 4-byte boundary. If the image has a width that is not divisible by four, say, 21 bytes, there would be 3 bytes of padding at the end of every scan line.
         */
        fseek(image_input, padding, SEEK_CUR);
        for (int i = 0; i < padding; i++) {
            fputc(0x00, image_output);
        }
    }
    
    fclose(image_input);
    fclose(image_output);
    
    return 0;
}
