/* stb_image - v2.27 - public domain single-file image loader
   Only include the header; implementation will be in texture.cpp
*/
#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_FAILURE_STRINGS

extern "C" {
    int stbi_info_from_memory(const unsigned char *buffer, int len, int *x, int *y, int *comp);
    unsigned char *stbi_load_from_memory(const unsigned char *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
    void stbi_image_free(void *retval_from_stbi_load);
}

#endif
