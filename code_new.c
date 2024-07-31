#include <math.h>
void depth_estimation(int img1[][256], int img2[][256], int img3[][256]) {

    int IMG_SIZE = 256;
    int MAX_DISP = 30;
    int WIN_SIZE = 5;
    
    int half_win = WIN_SIZE / 2;
    int i, j, d, x, y;

    double gaussian[WIN_SIZE][WIN_SIZE];
    double sigma = 1.0;
    double sum = 0.0;

    // Tính toán Gaussian kernel
    for (y = -half_win; y <= half_win; y++) {
        for (x = -half_win; x <= half_win; x++) {
            gaussian[y + half_win][x + half_win] = exp(-(x*x + y*y) / (2 * sigma * sigma));
            sum += gaussian[y + half_win][x + half_win];
        }
    }

    // Bình thường hóa Gaussian kernel
    for (y = 0; y < WIN_SIZE; y++) {
        for (x = 0; x < WIN_SIZE; x++) {
            gaussian[y][x] /= sum;
        }
    }

    #pragma omp parallel for private(i, j, d, x, y)
    for (j = half_win; j < IMG_SIZE - half_win; j++) {
        for (i = half_win; i < IMG_SIZE - half_win; i++) {
            int best_disparity = 0;
            double min_sad = 999999;

            for (d = 0; d <= MAX_DISP; d++) {
                int sad = 0;

                for (y = -half_win; y <= half_win; y++) {
                    for (x = -half_win; x <= half_win; x++) {
                        int ref_pixel = img1[j + y][i + x];
                        int tgt_pixel = (i + x + d < IMG_SIZE) ? img2[j + y][i + x + d] : 0;
                        sad += gaussian[y + half_win][x + half_win] * abs(ref_pixel - tgt_pixel);
                    }
                }

                if (sad < min_sad) {
                    min_sad = sad;
                    best_disparity = d;
                }
            }

            img3[j][i] = best_disparity;
        }
    }
}