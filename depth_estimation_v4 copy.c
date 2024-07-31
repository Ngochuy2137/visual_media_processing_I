#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define IMG_SIZE 256
#define DISPARITY_RANGE 30

void depth_estimation(int img1[IMG_SIZE][IMG_SIZE], int img2[IMG_SIZE][IMG_SIZE], int img3[IMG_SIZE][IMG_SIZE]) {
}

int main() {
    int img1[IMG_SIZE][IMG_SIZE];
    int img2[IMG_SIZE][IMG_SIZE];
    int img3[IMG_SIZE][IMG_SIZE];

    // Load images from files (example with dot01-1.pgm and dot01-2.pgm)
    FILE *f1 = fopen("/Users/huynn/Library/CloudStorage/OneDrive-Personal/WORK/3. DU HỌC/6. Master - NAIST/5. Study process/Sem-2/12. Visual media processing/Lec 8/depth_estimation_v4/dot2/dot01-1.pgm", "r");
    FILE *f2 = fopen("/Users/huynn/Library/CloudStorage/OneDrive-Personal/WORK/3. DU HỌC/6. Master - NAIST/5. Study process/Sem-2/12. Visual media processing/Lec 8/depth_estimation_v4/dot2/dot01-2.pgm", "r");
    // Skip the PGM header
    char header[20];
    fgets(header, sizeof(header), f1);
    fgets(header, sizeof(header), f2);
    fgets(header, sizeof(header), f1);
    fgets(header, sizeof(header), f2);

    // Read image data
    for (int i = 0; i < IMG_SIZE; i++) {
        for (int j = 0; j < IMG_SIZE; j++) {
            fscanf(f1, "%d", &img1[i][j]);
            fscanf(f2, "%d", &img2[i][j]);
        }
    }

    fclose(f1);
    fclose(f2);

    // Call depth estimation function
    depth_estimation(img1, img2, img3);

    // Save the resulting depth map (example with dot01_res.pgm)
    FILE *f3 = fopen("dot01_res.pgm", "w");
    fprintf(f3, "P2\n%d %d\n255\n", IMG_SIZE, IMG_SIZE);
    for (int i = 0; i < IMG_SIZE; i++) {
        for (int j = 0; j < IMG_SIZE; j++) {
            fprintf(f3, "%d ", img3[i][j]);
        }
        fprintf(f3, "\n");
    }
    fclose(f3);

    return 0;
}
