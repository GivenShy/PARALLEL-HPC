#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <immintrin.h>
#include <time.h>
#include <float.h>
#include <stdint.h>

#pragma pack(1)  

typedef struct {
    uint16_t type;      
    uint32_t size;      
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;    
} BMPHeader;

typedef struct {
    uint32_t size;      
    int32_t width;      
    int32_t height;     
    uint16_t planes;    
    uint16_t bitCount;  
    uint32_t compression; 
    uint32_t imageSize; 
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} DIBHeader;

void iter_convert(uint8_t *pixelData, int width, int height, int rowSize) {
    for (int y = 0; y < height; y++) {
        int rowStart = y * rowSize;
        for (int x = 0; x < width * 3; x += 3) {
            uint8_t B = pixelData[rowStart + x];     
            uint8_t G = pixelData[rowStart + x + 1]; 
            uint8_t R = pixelData[rowStart + x + 2]; 

            uint8_t gray = (uint8_t)(0.299 * R + 0.587 * G + 0.114 * B);

            pixelData[rowStart + x] = gray;     
            pixelData[rowStart + x + 1] = gray;
            pixelData[rowStart + x + 2] = gray; 
        }
    }
}

void grayscale_simd(uint8_t *data, int width, int height, int rowSize) {
    const __m256 weightR = _mm256_set1_ps(0.299f);
    const __m256 weightG = _mm256_set1_ps(0.587f);
    const __m256 weightB = _mm256_set1_ps(0.114f);
    
    for (int y = 0; y < height; y++) {
        const int rowStart = y * rowSize;
        int x;
        
        for (x = 0; x < width * 3 - 24; x += 24) {
            uint8_t *pixel = data + rowStart + x;
            
            float r_data[8], g_data[8], b_data[8];
            
            for (int i = 0; i < 8; i++) {
                b_data[i] = pixel[i*3];    
                g_data[i] = pixel[i*3 + 1]; 
                r_data[i] = pixel[i*3 + 2]; 
            }
            
            __m256 r = _mm256_loadu_ps(r_data);
            __m256 g = _mm256_loadu_ps(g_data);
            __m256 b = _mm256_loadu_ps(b_data);
            

            __m256 r_component = _mm256_mul_ps(weightR, r);
            __m256 g_component = _mm256_mul_ps(weightG, g);
            __m256 b_component = _mm256_mul_ps(weightB, b);
            
            __m256 rg_sum = _mm256_add_ps(r_component, g_component);
            __m256 sum = _mm256_add_ps(rg_sum, b_component);
            
            __m256i intValues = _mm256_cvttps_epi32(sum);
            
            uint8_t grayValues[8];
            
            __m128i low = _mm_packus_epi32(_mm256_extracti128_si256(intValues, 0),
                                          _mm256_extracti128_si256(intValues, 1));
            __m128i packed = _mm_packus_epi16(low, _mm_setzero_si128());
            
            _mm_storeu_si128((__m128i*)grayValues, packed);
            
            for (int i = 0; i < 8; i++) {
                pixel[i*3]     = grayValues[i]; 
                pixel[i*3 + 1] = grayValues[i]; 
                pixel[i*3 + 2] = grayValues[i]; 
            }
        }
        
        for (; x < width * 3; x += 3) {
            uint8_t B = data[rowStart + x];     
            uint8_t G = data[rowStart + x + 1]; 
            uint8_t R = data[rowStart + x + 2];

            uint8_t gray = (uint8_t)(0.299f * R + 0.587f * G + 0.114f * B);

            data[rowStart + x]     = gray; 
            data[rowStart + x + 1] = gray;
            data[rowStart + x + 2] = gray; 
        }
    }
}

int main() {
    const char *inputFile = "5184x3456.bmp";  
    const char *outputFile = "output_grayscale_iter.bmp"; 
    const char *outputFile_simd = "output_grayscale_simd.bmp";
    clock_t start, end;
    FILE *file = fopen(inputFile, "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    BMPHeader bmpHeader;
    DIBHeader dibHeader;

    // Read BMP headers
    fread(&bmpHeader, sizeof(BMPHeader), 1, file);
    fread(&dibHeader, sizeof(DIBHeader), 1, file);

    int extraHeaderBytes = bmpHeader.offset - sizeof(BMPHeader) - sizeof(DIBHeader);
    uint8_t *extraHeaderData = NULL;
    if (extraHeaderBytes > 0) {
        extraHeaderData = (uint8_t *)malloc(extraHeaderBytes);
        fread(extraHeaderData, extraHeaderBytes, 1, file);
    }

    if (bmpHeader.type != 0x4D42 || dibHeader.bitCount != 24) {
        printf("Not a valid 24-bit BMP file!\n");
        fclose(file);
        return 1;
    }

    printf("Image Width: %d px\n", dibHeader.width);
    printf("Image Height: %d px\n", dibHeader.height);
    printf("Bits Per Pixel: %d\n", dibHeader.bitCount);
    printf("Pixel Data Offset: %d\n", bmpHeader.offset);

    fseek(file, bmpHeader.offset, SEEK_SET);
    int rowSize;
    int temp = dibHeader.width * 3;
    if(temp%4==0){
        rowSize = temp;
    }
    else{
        rowSize = temp+(4-temp%4);
    }
    int imgSize = rowSize * dibHeader.height;
    uint8_t *pixelData = (uint8_t *)malloc(imgSize);
    uint8_t *pixelData_simd = (uint8_t *)malloc(imgSize);
    if (!pixelData) {
        printf("Memory allocation failed!\n");
        fclose(file);
        return 1;
    }
    
    fread(pixelData, imgSize, 1, file);
    fclose(file);
    memcpy(pixelData_simd, pixelData, imgSize);

    start = clock();
    iter_convert(pixelData, dibHeader.width, dibHeader.height, rowSize);
    end = clock();
    double time_simd = ((double)(end - start));
    printf("Iter Execution Time: %f\n", time_simd);

    start = clock();
    grayscale_simd(pixelData_simd, dibHeader.width, dibHeader.height, rowSize);
    end = clock();
    time_simd = ((double)(end - start));
    printf("SIMD Execution Time: %f\n", time_simd);

    bmpHeader.size = bmpHeader.offset + imgSize;
    dibHeader.imageSize = imgSize;

    FILE *output = fopen(outputFile, "wb");
    if (!output) {
        perror("Error opening output file");
        free(pixelData);
        return 1;
    }

    fwrite(&bmpHeader, sizeof(BMPHeader), 1, output);
    fwrite(&dibHeader, sizeof(DIBHeader), 1, output);

    if (extraHeaderBytes > 0) {
        fwrite(extraHeaderData, extraHeaderBytes, 1, output);
    }

    fwrite(pixelData, imgSize, 1, output);

    fclose(output);
    free(pixelData);

    FILE *output_simd = fopen(outputFile_simd, "wb");
    if (!output_simd) {
        perror("Error opening output file");
        free(pixelData_simd);
        return 1;
    }

    fwrite(&bmpHeader, sizeof(BMPHeader), 1, output_simd);
    fwrite(&dibHeader, sizeof(DIBHeader), 1, output_simd);
    if (extraHeaderBytes > 0) {
        fwrite(extraHeaderData, extraHeaderBytes, 1, output_simd);
        free(extraHeaderData);
    }
    fwrite(pixelData_simd, imgSize, 1, output_simd);
    fclose(output_simd);
    free(pixelData_simd);

    return 0;
}

