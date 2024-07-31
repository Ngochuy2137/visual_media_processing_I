#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
// #include <math.h>

int IMG_SIZE = 256;
int MAX_DISP = 30;

// void depth_estimation(int img1[IMG_SIZE][IMG_SIZE], int img2[IMG_SIZE][IMG_SIZE], int img3[IMG_SIZE][IMG_SIZE]) {
//     for (int i = 10; i < IMG_SIZE - 10; i++) {
//         for (int j = 10; j < IMG_SIZE - 10; j++) {
//             int min_ssd = INT_MAX;
//             int best_disparity = 0;
//             for (int d = 0; d < MAX_DISP; d++) {
//                 if (j - d < 10) continue;
//                 int ssd = 0;
//                 for (int k = -1; k <= 1; k++) {
//                     for (int l = -1; l <= 1; l++) {
//                         int diff = img1[i + k][j + l] - img2[i + k][j + l - d];
//                         ssd += diff * diff;
//                     }
//                 }
//                 if (ssd < min_ssd) {
//                     min_ssd = ssd;
//                     best_disparity = d;
//                 }
//             }
//             img3[i][j] = best_disparity;
//         }
//     }
// }

// --------------------------------------------- New code ---------------------------------------------

void depth_estimation(int img1[256][256], int img2[256][256], int img3[256][256]){
    int IMG_SIZE = 256;
    int IGNORE_SIZE = 16;
    int WIN_SIZE = 16;
    int MAX_DISP = 30;

    int half_win = WIN_SIZE / 2;
    int i, j, d, x, y;
    int a = 0;
    #pragma omp parallel for private(i, j, d, x, y)
    for (j = IGNORE_SIZE+half_win; j < IMG_SIZE - (IGNORE_SIZE+half_win); j++) {
        for (i = IGNORE_SIZE+half_win; i < IMG_SIZE - (IGNORE_SIZE+half_win); i++) {
            int best_disparity = 0;
            int min_sad = 999999;

            for (d = 0; d <= MAX_DISP; d++) {
                int sad = 0;

                for (y = -half_win; y <= half_win; y++) {
                    for (x = -half_win; x <= half_win; x++) {
                        int ref_pixel = img1[j + y][i + x];
                        int tgt_pixel = (i + x + d < IMG_SIZE) ? img2[j + y][i + x + d] : 0;
                        sad += abs(ref_pixel - tgt_pixel);
                    }
                }

                if (sad < min_sad) {
                    min_sad = sad;
                    best_disparity = d;
                    if (d != 0) {
                        a = d;
                        printf("a = %d\n", a);
                    }
                }
            }

            img3[j][i] = best_disparity;
        }
    }
}


// --------------------------------------------- Nam code ---------------------------------------------

// #include <math.h>
// void depth_estimation(int img1[][256], int img2[][256], int img3[][256]){
//     int IMG_SIZE = 256;
//     int WIN_SIZE = 5;
//     int MAX_DISP = 30;

//     int half_win = WIN_SIZE / 2;
//     int i, j, d, x, y;

//     #pragma omp parallel for private(i, j, d, x, y)
//     for (j = half_win; j < IMG_SIZE - half_win; j++) {
//         for (i = half_win; i < IMG_SIZE - half_win; i++) {
//             int best_disparity = 0;
//             int min_sad = 999999;

//             for (d = 0; d <= MAX_DISP; d++) {
//                 int sad = 0;

//                 for (y = -half_win; y <= half_win; y++) {
//                     for (x = -half_win; x <= half_win; x++) {
//                         int ref_pixel = img1[j + y][i + x];
//                         int tgt_pixel = (i + x + d < IMG_SIZE) ? img2[j + y][i + x + d] : 0;
//                         sad += abs(ref_pixel - tgt_pixel);
//                     }
//                 }

//                 if (sad < min_sad) {
//                     min_sad = sad;
//                     best_disparity = d;
//                 }
//             }

//             img3[j][i] = best_disparity;
//         }
//     }
// }
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
