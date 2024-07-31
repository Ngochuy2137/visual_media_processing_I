#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define IMG_SIZE 256
#define MAX_DISP 30

// Các phương pháp tính toán
enum Method { DEFAULT, SAD, SSD, NCC };

// Hàm đọc ảnh
void img_in(const char *n, int img[][IMG_SIZE]) {
    FILE *fp;
    int i, j;
    char tmps[256], str[4024], *tok;

    if ((fp = fopen(n, "r")) == NULL) {
        printf("Cannot open %s file!!\n", n);
        exit(0);
    };
    fscanf(fp, "%s", tmps);
    fscanf(fp, "%d %d", &i, &j);
    fscanf(fp, "%d ", &i);
    j = 0;
    while (fgets(str, 4024, fp) != NULL) {
        tok = strtok(str, " ");
        i = 0;
        img[j][i++] = atoi(tok);
        while (tok != NULL) {
            img[j][i] = atoi(tok);
            tok = strtok(NULL, " \n");
            i++;
        }
        j++;
    }
    fclose(fp);
}

// Hàm ghi ảnh
void img_out(const char *n, int img[][IMG_SIZE]) {
    FILE *fp;
    int i, j;

    fp = fopen(n, "w");
    fprintf(fp, "P2\n256 256\n255\n");
    for (j = 0; j < IMG_SIZE; j++) {
        for (i = 0; i < IMG_SIZE; i++) {
            fprintf(fp, "%d ", img[j][i]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

// Hàm tính toán disparity với các phương pháp khác nhau
// Hàm tính toán disparity với các phương pháp khác nhau
void depth_estimation(int img1[][IMG_SIZE], int img2[][IMG_SIZE], int img3[][IMG_SIZE], enum Method method, int win_size) {
    int i, j, a, b;
    int d, d2, dt;
    int half_win = win_size / 2;
    double ncc_num, ncc_den1, ncc_den2, mean1, mean2;
    int count;

    // Tạo mảng trọng số Gaussian
    double gaussian[win_size][win_size];
    double sigma = win_size / 2.0;
    double sum = 0.0;

    for (b = -half_win; b <= half_win; b++) {
        for (a = -half_win; a <= half_win; a++) {
            double distance = sqrt(a * a + b * b);
            gaussian[b + half_win][a + half_win] = exp(-(distance * distance) / (2 * sigma * sigma));
            sum += gaussian[b + half_win][a + half_win];
        }
    }

    for (b = 0; b < win_size; b++) {
        for (a = 0; a < win_size; a++) {
            gaussian[b][a] /= sum;
        }
    }

    #pragma omp parallel for private(i, j, a, b, d, d2, dt, ncc_num, ncc_den1, ncc_den2, mean1, mean2, count)
    for (j = half_win; j < IMG_SIZE - half_win; j++) {
        for (i = half_win; i < IMG_SIZE - half_win; i++) {
            d2 = 0;
            int dmin = 999999;
            double max_ncc = -1;

            for (d = 0; d <= MAX_DISP; d++) {
                dt = 0;
                ncc_num = 0;
                ncc_den1 = 0;
                ncc_den2 = 0;
                mean1 = 0;
                mean2 = 0;
                count = 0;

                // Tính giá trị trung bình của các pixel trong cửa sổ
                for (b = -half_win; b <= half_win; b++) {
                    for (a = -half_win; a <= half_win; a++) {
                        int ref_pixel = img1[j + b][i + a];
                        int tgt_pixel = (i + a + d < IMG_SIZE) ? img2[j + b][i + a + d] : 0;
                        mean1 += ref_pixel;
                        mean2 += tgt_pixel;
                        count++;
                    }
                }
                mean1 /= count;
                mean2 /= count;

                // Tính toán giá trị NCC
                for (b = -half_win; b <= half_win; b++) {
                    for (a = -half_win; a <= half_win; a++) {
                        int ref_pixel = img1[j + b][i + a];
                        int tgt_pixel = (i + a + d < IMG_SIZE) ? img2[j + b][i + a + d] : 0;
                        double weight = gaussian[b + half_win][a + half_win];

                        if (method == DEFAULT) {
                            if (ref_pixel != tgt_pixel) {
                                dt++;
                            }
                        } else if (method == SAD) {
                            dt += weight * abs(ref_pixel - tgt_pixel);
                        } else if (method == SSD) {
                            dt += weight * (ref_pixel - tgt_pixel) * (ref_pixel - tgt_pixel);
                        } else if (method == NCC) {
                            ncc_num += weight * (ref_pixel - mean1) * (tgt_pixel - mean2);
                            ncc_den1 += weight * (ref_pixel - mean1) * (ref_pixel - mean1);
                            ncc_den2 += weight * (tgt_pixel - mean2) * (tgt_pixel - mean2);
                        }
                    }
                }

                if (method == NCC) {
                    double ncc = ncc_num / sqrt(ncc_den1 * ncc_den2);
                    if (ncc > max_ncc) {
                        max_ncc = ncc;
                        d2 = d;
                    }
                } else {
                    if (dmin > dt) {
                        d2 = d;
                        dmin = dt;
                    }
                }
            }
            img3[j][i] = d2;
        }
    }
}

int main() {
    // int i, j, k, error = 0;
    // char str[256] = {0};
    // double ave_err = 0, ave_time = 0;

    // int fnum = 10;  /* 画像ペア数　入力ファイルの数を変更した場合ここも修正が必要　*/
    // char imname[][256] = {"", "dot01", "dot02", "dot03", "dot04", "dot05", "dot06", "dot07", "dot08", "dot09", "dot10"}; /* 入力ステレオ画像名*/

    // int img1[IMG_SIZE][IMG_SIZE], img2[IMG_SIZE][IMG_SIZE]; /* 入力ステレオ画像*/
    // int img_gt[IMG_SIZE][IMG_SIZE]; /* 正解データ画像*/
    // int img3[IMG_SIZE][IMG_SIZE] = {0}; /* 推定結果画像*/

    enum Method methods[] = { DEFAULT, SAD, SSD, NCC };
    int win_sizes[] = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20}; // Các kích thước cửa sổ

    // enum Method methods[] = {SAD};
    // int win_sizes[] = {16}; // Các kích thước cửa sổ


    for (int m = 0; m < sizeof(methods) / sizeof(methods[0]); m++) {
        enum Method method = methods[m];
        printf("\n\n========================== METHOD %d======================================", method);
        for (int w = 0; w < sizeof(win_sizes) / sizeof(win_sizes[0]); w++) {
            int win_size = win_sizes[w];
            printf("\n        ---------------------------- Window size: %d\n", win_size);


            int i,j,k,error=0;
            char str[256]={0};
            double ave_err=0,ave_time=0;

            int fnum=10;  /* 画像ペア数　入力ファイルの数を変更した場合ここも修正が必要　*/ 
            char imname[][256]={"","dot01","dot02","dot03","dot04","dot05","dot06","dot07","dot08","dot09","dot10"}; /* 入力ステレオ画像名*/


            int img1[256][256],img2[256][256]; /* 入力ステレオ画像*/
            int img_gt[256][256]; /* 正解データ画像*/
            int img3[256][256]={0}; /* 推定結果画像*/
            
            clock_t start_clock, end_clock;

            k=1;


            while (k <= fnum) {
                strcpy(str, "/home/huynn/Downloads/Lec_8/Lec_8/depth_estimation_v4/dot2/");
                strcat(str, imname[k]);
                img_in(strcat(str, "-1.pgm"), img1);

                strcpy(str, "/home/huynn/Downloads/Lec_8/Lec_8/depth_estimation_v4/dot2/");
                strcat(str, imname[k]);
                img_in(strcat(str, "-2.pgm"), img2);

                strcpy(str, "/home/huynn/Downloads/Lec_8/Lec_8/depth_estimation_v4/dot2/");
                strcat(str, imname[k]);
                img_in(strcat(str, "-gt.pgm"), img_gt);

                start_clock = clock();

                depth_estimation(img1, img2, img3, method, win_size);

                end_clock = clock();

                /* 正答率判定　*/
                error = 0;
                for (j=0;j<256;j++){
                    for (i=0;i<256;i++){
                        if (img3[j][i] != img_gt[j][i] )error ++; 
                    }
                }

                // printf("    %s -> Error Rate: %lf, Time: %f\n", imname[k], error / (j * i * 1.0), (double)(end_clock - start_clock) / CLOCKS_PER_SEC);
                strcpy(str, "");
                strcat(str, imname[k]);
                img_out(strcat(str, "-res.pgm"), img3);
                ave_err += error / (j * i * 1.0);
                ave_time += (double)(end_clock - start_clock) / CLOCKS_PER_SEC;
                k++;
            }
            // print k
            printf("    ave_err: %lf\n", ave_err);
            printf("    (k - 1): %d\n", (k - 1));
            printf("Method: %d, Window Size: %d, Average Error Rate: %lf, Average Time: %lf\n", method, win_size, ave_err / (k - 1), ave_time / (k - 1));
        }
    }

    return 0;
}
