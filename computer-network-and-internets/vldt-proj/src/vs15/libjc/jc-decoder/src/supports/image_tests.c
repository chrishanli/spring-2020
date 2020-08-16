// image_tests.c    09/03/2020
// J_Code
// brief: 备用函数，以防OpenCV出问题。应用libpng 1.6库
// Han Li

#include <string.h> // memset
#include <jc.h>
#include <libpng/png.h>

/**
 Save code bitmap in RGB as png image
 
 @param bitmap the code bitmap
 @param filename filename the image filename
 @return SUCCESS | FAILURE
 */
Boolean saveImage(Bitmap* bitmap, Char* filename)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    
    if(bitmap->channel_count == 4)
    {
        image.format = PNG_FORMAT_RGBA;
        image.flags  = PNG_FORMAT_FLAG_ALPHA | PNG_FORMAT_FLAG_COLOR;
    }
    else
    {
        image.format = PNG_FORMAT_GRAY;
    }
    
    image.width  = bitmap->width;
    image.height = bitmap->height;
    
    if (png_image_write_to_file(&image,
                                filename,
                                0/*convert_to_8bit*/,
                                bitmap->pixel,
                                0/*row_stride*/,
                                NULL/*colormap*/) == 0)
    {
        J_REPORT_ERROR((image.message))
        J_REPORT_ERROR(("Saving png image failed"))
        return FAILURE;
    }
    return SUCCESS;
}

