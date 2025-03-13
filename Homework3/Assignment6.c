#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <immintrin.h>
#include <time.h>
#include <float.h>
#include <stdint.h>

#pragma pack(1)  // Ensure correct structure alignment

// BMP File Header (14 bytes)
typedef struct {
    uint16_t type;      // BMP Identifier ('BM' = 0x4D42)
    uint32_t size;      // File size in bytes
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;    // Offset where pixel data starts
} BMPHeader;

// BMP Info Header (DIB Header, Variable Size)
typedef struct {
    uint32_t size;      // Header size (40 bytes for basic BMP)
    int32_t width;      // Image width
    int32_t height;     // Image height
    uint16_t planes;    // Color planes (must be 1)
    uint16_t bitCount;  // Bits per pixel (24 for BGR)
    uint32_t compression; // Compression (0 = uncompressed)
    uint32_t imageSize; // Image data size (can be 0 if uncompressed)
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} DIBHeader;

// Convert BMP to grayscale (Fixes padding issues)
void iter_convert(uint8_t *pixelData, int width, int height, int rowSize) {
    for (int y = 0; y < height; y++) {
        int rowStart = y * rowSize;
        for (int x = 0; x < width * 3; x += 3) {
            uint8_t B = pixelData[rowStart + x];     // Blue channel
            uint8_t G = pixelData[rowStart + x + 1]; // Green channel
            uint8_t R = pixelData[rowStart + x + 2]; // Red channel

            // Compute grayscale value
            uint8_t gray = (uint8_t)(0.299 * R + 0.587 * G + 0.114 * B);

            // Set all channels to grayscale
            pixelData[rowStart + x] = gray;     // B
            pixelData[rowStart + x + 1] = gray; // G
            pixelData[rowStart + x + 2] = gray; // R
        }
    }
}


void grayscale_simd(uint8_t *data, int width, int height, int rowSize) {
    __m256 weightR = _mm256_set1_ps(0.299f);
    __m256 weightG = _mm256_set1_ps(0.587f);
    __m256 weightB = _mm256_set1_ps(0.114f);

    for (int y = 0; y < height; y++) {
        int rowStart = y * rowSize;
        int x;
        for (x = 0; x < width * 3-24; x += 24) {
            __m256 r = _mm256_setr_ps(data[rowStart + x+2],data[rowStart + x+5],data[rowStart + x+8],data[rowStart + x+11],
                data[rowStart + x+14],data[rowStart + x+17],data[rowStart + x+20],data[rowStart + x+23]);
            __m256 g = _mm256_setr_ps(data[rowStart + x+1],data[rowStart + x+4],data[rowStart + x+7],data[rowStart + x+10],
                data[rowStart + x+13],data[rowStart + x+16],data[rowStart + x+19],data[rowStart + x+22]);  
            __m256 b = _mm256_setr_ps(data[rowStart + x],data[rowStart + x+3],data[rowStart + x+6],data[rowStart + x+9],
                data[rowStart + x+12],data[rowStart + x+15],data[rowStart + x+18],data[rowStart + x+21]); 
            
            __m256 sum = _mm256_add_ps(_mm256_mul_ps(weightR,r),_mm256_add_ps(_mm256_mul_ps(weightG,g),_mm256_mul_ps(weightB,b)));
            __m256i intValues = _mm256_cvtps_epi32(sum);
            int values[8];  // Array to store 8 integers
            _mm256_storeu_si256((__m256i*)values, intValues);
            for(int i = 0;i<8;i++){
                for(int j = 0;j<3;j++){
                    data[rowStart+x+i*3+j] = values[i];
                }
            }

        }
        for (; x < width * 3; x += 3) {
            uint8_t B = data[rowStart + x];     // Blue channel
            uint8_t G = data[rowStart + x + 1]; // Green channel
            uint8_t R = data[rowStart + x + 2]; // Red channel

            // Compute grayscale value
            uint8_t gray = (uint8_t)(0.299 * R + 0.587 * G + 0.114 * B);

            // Set all channels to grayscale
            data[rowStart + x] = gray;     // B
            data[rowStart + x + 1] = gray; // G
            data[rowStart + x + 2] = gray; // R
        }
    }
}

int main() {
    const char *inputFile = "640x426.bmp";  // Input BMP file
    const char *outputFile = "output_grayscale_iter.bmp"; // Output file
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

    // Read additional metadata if offset > 54
    int extraHeaderBytes = bmpHeader.offset - sizeof(BMPHeader) - sizeof(DIBHeader);
    uint8_t *extraHeaderData = NULL;
    if (extraHeaderBytes > 0) {
        extraHeaderData = (uint8_t *)malloc(extraHeaderBytes);
        fread(extraHeaderData, extraHeaderBytes, 1, file);
    }

    // Validate BMP format
    if (bmpHeader.type != 0x4D42 || dibHeader.bitCount != 24) {
        printf("Not a valid 24-bit BMP file!\n");
        fclose(file);
        return 1;
    }

    printf("Image Width: %d px\n", dibHeader.width);
    printf("Image Height: %d px\n", dibHeader.height);
    printf("Bits Per Pixel: %d\n", dibHeader.bitCount);
    printf("Pixel Data Offset: %d\n", bmpHeader.offset);

    // Seek to pixel data
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
    
    // Read pixel data
    fread(pixelData, imgSize, 1, file);
    fclose(file);
    memcpy(pixelData_simd, pixelData, imgSize);

    start = clock();
    iter_convert(pixelData, dibHeader.width, dibHeader.height, rowSize);
    end = clock();
    double time_simd = ((double)(end - start));
    printf("Iter Execution Time: %f sec\n", time_simd);

    start = clock();
    grayscale_simd(pixelData_simd, dibHeader.width, dibHeader.height, rowSize);
    end = clock();
    time_simd = ((double)(end - start));
    printf("SIMD Execution Time: %f sec\n", time_simd);

    // Update BMP file size in header
    bmpHeader.size = bmpHeader.offset + imgSize;
    dibHeader.imageSize = imgSize;

    // Save the modified image
    FILE *output = fopen(outputFile, "wb");
    if (!output) {
        perror("Error opening output file");
        free(pixelData);
        return 1;
    }

    // Write BMP headers
    fwrite(&bmpHeader, sizeof(BMPHeader), 1, output);
    fwrite(&dibHeader, sizeof(DIBHeader), 1, output);

    // Write extra metadata if present
    if (extraHeaderBytes > 0) {
        fwrite(extraHeaderData, extraHeaderBytes, 1, output);
    }

    // Write modified pixel data
    fwrite(pixelData, imgSize, 1, output);

    fclose(output);
    free(pixelData);

    FILE *output_simd = fopen(outputFile_simd, "wb");
    if (!output_simd) {
        perror("Error opening output file");
        free(pixelData_simd);
        return 1;
    }

    // Write BMP headers
    fwrite(&bmpHeader, sizeof(BMPHeader), 1, output_simd);
    fwrite(&dibHeader, sizeof(DIBHeader), 1, output_simd);
    // Write extra metadata if present
    if (extraHeaderBytes > 0) {
        fwrite(extraHeaderData, extraHeaderBytes, 1, output_simd);
        free(extraHeaderData);
    }
    // Write modified pixel data
    fwrite(pixelData_simd, imgSize, 1, output_simd);
    fclose(output_simd);
    free(pixelData_simd);

    return 0;
}

