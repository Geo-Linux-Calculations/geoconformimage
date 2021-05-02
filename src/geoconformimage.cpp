/*
** geoconform.c
**
** Public Domain Mark 1.0
** No Copyright
**
** Depends:
** 1) lm
** 2) freeimage
*/

#include <unistd.h>
#include <FreeImage.h>
#include "geoconformfreeimage.h"

////////////////////////////////////////////////////////////////////////////////

void GeoConformImageTitle()
{
    printf("GeoConformImage.\n");
    printf("https://github.com/Geo-Linux-Calculations/geoconformimage/.\n\n");
}

////////////////////////////////////////////////////////////////////////////////

void GeoConformImageUsage(const char *progname)
{
    printf("Usage : %s [options] <input_image> <output_image>\n", progname);
    printf("options:\n");
    printf("    -i N    iteration set (default = %d)\n", COUNTG);
    printf("    -m N    margin (default = %d)\n", COUNTM);
    printf("    -p str  string conform params: \"A0,B0,A1,B1,[...,A9,B9]\"\n");
    printf("    -r str  string region image: \"Xws,Yws,Xne,Yne\"\n");
    printf("    -h      this help\n");
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    // call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
    FreeImage_Initialise();
#endif // FREEIMAGE_LIB

    int opt, nk = 0;
    GCIparams params;
    IMTimage imgin, imgout;
    bool fhelp = false;
    params.iters = COUNTG;
    params.margin = COUNTM;

    GeoConformImageTitle();

    while ((opt = getopt(argc, argv, ":i:m:p:r:h")) != -1)
    {
        switch(opt)
        {
        case 'i':
            params.iters = atoi(optarg);
            if (params.iters < 2) params.iters = 2;
            printf("Parameter iters set %d.\n", params.iters);
            break;
        case 'm':
            params.margin = atoi(optarg);
            if (params.margin < 0) params.margin = 0;
            printf("Parameter margin set %d.\n", params.margin);
            break;
        case 'p':
            params.trans.na = sscanf(optarg, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &params.trans.a[0], &params.trans.a[1], &params.trans.a[2], &params.trans.a[3], &params.trans.a[4], &params.trans.a[5], &params.trans.a[6], &params.trans.a[7], &params.trans.a[8], &params.trans.a[9], &params.trans.a[10], &params.trans.a[11], &params.trans.a[12], &params.trans.a[13], &params.trans.a[14], &params.trans.a[15], &params.trans.a[16], &params.trans.a[17], &params.trans.a[18], &params.trans.a[19]);
            if (params.trans.na < 3)
            {
                fprintf(stderr, "Error! Bad parameters: %s (%d < %d).\n", optarg, params.trans.na, 3);
                exit(1);
            }
            printf("Parameter trans set %s (%d).\n", optarg, params.trans.na);
            break;
        case 'r':
            nk = sscanf(optarg, "%f,%f,%f,%f", &params.rect1.p[0].x, &params.rect1.p[0].y, &params.rect1.p[2].x, &params.rect1.p[2].y);
            if (nk < 4)
            {
                fprintf(stderr, "Error! Bad region: %s (%d < %d).\n", optarg, nk, 4);
                exit(1);
            }
            printf("Parameter rect set %s (%d).\n", optarg, nk);
            break;
        case 'h':
            fhelp = true;
            break;
        case ':':
            printf("option needs a value\n");
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }

    if (optind + 2 > argc || params.trans.na < 3 || nk < 4 || fhelp)
    {
        GeoConformImageUsage(argv[0]);
        return 0;
    }
    const char *src_filename = argv[optind];
    const char *output_filename = argv[optind + 1];

    FreeImage_SetOutputMessage(FreeImageErrorHandler);

    printf("Input= %s\n", src_filename);
    FIBITMAP *dib = ImthresholdGenericLoader(src_filename, 0);
    if (dib)
    {
        if (FreeImage_GetImageType(dib) == FIT_BITMAP)
        {
            FIBITMAP* dst_dib;
            params.size1.width = FreeImage_GetWidth(dib);
            params.size1.height = FreeImage_GetHeight(dib);
            imgin = IMTalloc(params.size1, 24);
            imgin = ImthresholdGetData(dib, imgin);
            FreeImage_Unload(dib);
            params = GCIcalcallparams(params);
            imgout = IMTalloc(params.size2, 24);
            printf("Result image size \"%dx%d\".\n", params.size2.width, params.size2.height);

            IMTFilterGeoConform(imgin, imgout, params);
            imgin = IMTfree(imgin);
            dst_dib = FreeImage_Allocate(imgout.size.width, imgout.size.height, 24);
            ImthresholdSetData(dst_dib, imgout);
            imgout = IMTfree(imgout);
            printf("Result rect from \"(%f,%f)-(%f,%f)\" to \"(%f,%f)-(%f,%f)\".\n", params.rect1.min.x, params.rect1.min.y, params.rect1.max.x, params.rect1.max.y, params.rect2.min.x, params.rect2.min.y, params.rect2.max.x, params.rect2.max.y);

            if (dst_dib)
            {
                FREE_IMAGE_FORMAT out_fif = FreeImage_GetFIFFromFilename(output_filename);
                if(out_fif != FIF_UNKNOWN)
                {
                    FreeImage_Save(out_fif, dst_dib, output_filename, 0);
                    printf("Output= %s\n\n", output_filename);
                }
                FreeImage_Unload(dst_dib);
            }
        }
        else
        {
            printf("%s\n", "Unsupported format type.");
            FreeImage_Unload(dib);
        }
    }

    // call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
    FreeImage_DeInitialise();
#endif // FREEIMAGE_LIB

    return 0;
}
