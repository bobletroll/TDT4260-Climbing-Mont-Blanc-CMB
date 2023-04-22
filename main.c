// DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//                    Version 2, April 2023
//  
// Copyright (C) 2024 strawberryhacker
// 
// Everyone is permitted to copy and distribute verbatim or modified
// copies of this license document, and changing it is allowed as long
// as the name is changed.
//  
//            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
// 
//  0. You just DO WHAT THE FUCK YOU WANT TO.

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <string.h>

typedef float v4 __attribute__((vector_size(16)));
typedef struct {unsigned char x, y, z; } v3;

v3* read_image(const char* path, int* out_w, int* out_h) {
  FILE* file = path ? fopen(path, "rb") : stdin;
  assert(file);
  char buffer[16];
  assert(fgets(buffer, sizeof(buffer), file));
  assert(buffer[0] == 'P' && buffer[1] == '6');
  
  while (getc(file) == '#')
    while (getc(file) != '\n');
  fseek(file, -1, SEEK_CUR);

  int w, h, d;
  assert(fscanf(file, "%d %d\n%d\n", &w, &h, &d) == 3);
  assert(d == 255);

  v3* data = malloc(w * h * sizeof(v3));
  assert(fread(data, w * h * sizeof(v3), 1, file) == 1);
  if (path) fclose(file);

  *out_h = h;
  *out_w = w;
  return data;
}

void write_image(const char* path, v3* data, int w, int h) {
  FILE* file = path ? fopen(path, "wb") : stdout;
  assert(file);
  fprintf(file, "P6\n# Created by RPFELGUEIRAS\n%d %d\n255\n", w, h);
  fwrite(data, w * h * sizeof(v3), 1, file);
  if (path) fclose(file);
}

void blur(v4* input_data, v4* input_buffer, int w, int h, int R) {
  v4 (*data)   [w] = (void*) input_data;
  v4 (*buffer) [h] = (void*) input_buffer; // Transposed

  #pragma omp parallel for simd
  for (int y = 0; y < h; y++) {
    v4 sum = {0};
    int x;
    
    for (x = 0; x < R; x++)
      sum += data[y][x];
    
    for (x = 0; x < R; x++) {
      sum += data[y][x + R];
      buffer[x][y] = sum / (float) (R + x + 1);
    }

    for (; x < w - R; x++) {
      sum += data[y][x + R];
      buffer[x][y] = sum / (float) (2 * R + 1);
      sum -= data[y][x - R];
    }

    for (int i = 0; x < w; x++, i++) {
      buffer[x][y] = sum / (float) (2 * R - i);
      sum -= data[y][x - R];
    }
  }

  #pragma omp parallel for simd
  for (int x = 0; x < w; x++) {
    v4 sum = {0};
    int y;

    for (y = 0; y < R; y++)
      sum += buffer[x][y];
    
    for (y = 0; y < R; y++) {
      sum += buffer[x][y + R];
      data[y][x] = sum / (float) (R + y + 1);
    }

    for (; y < h - R; y++) {
      sum += buffer[x][y + R];
      data[y][x] = sum / (float) (2 * R + 1);
      sum -= buffer[x][y - R];
    }

    for (int i = 0; y < h; y++, i++) {
      data[y][x] = sum / (float) (2 * R - i);
      sum -= buffer[x][y - R];
    }
  }
}

v3* difference(v4* input_small, v4* input_large, int w, int h) {
  v3* output_data = malloc(w * h * sizeof(v3));

  #pragma omp parallel for simd
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      float* small = (float*) &input_small[i * w + j];
      float* large = (float*) &input_large[i * w + j];
      unsigned char* output = &output_data[i * w + j].x;

      for (int c = 0; c < 3; c++) {
        float diff = large[c] - small[c];
        if (diff < -1.0)
          diff += 257.0;
        output[c] = truncf(diff);
      }
    }
  }

  return output_data;
}

static void check(char* path, char* correct_path) {
  int w, h;
  v3* correct_data = read_image(correct_path, &w, &h);
  v3* data         = read_image(path,         &w, &h);

  int errors[256] = {0};
  for (int i = 0; i < w * h; i++) {
    errors[(unsigned char) ((int) data[i].x - correct_data[i].x)]++;
    errors[(unsigned char) ((int) data[i].y - correct_data[i].y)]++;
    errors[(unsigned char) ((int) data[i].z - correct_data[i].z)]++;
  }

  int single_errors = errors[1] + errors[(unsigned char) -1];
  int multi_errors = w * h * sizeof(v3) - (errors[0] + single_errors);
  printf("%-25s :  single:%-10d  multi:%-10d\n", path, single_errors, multi_errors);
}

int main(int count, char** arguments) {
  int w, h;
  v3* image_data = read_image(count > 1 ? "data/flower.ppm" : 0, &w, &h);

  int kernels[4] = {2, 3, 5, 8};
  v4* memory = malloc(8 * w * h * sizeof(v4));
  v4* data[4];
  v4* buffer[4];

  for (int i = 0; i < 4; i++) {
    data[i]   = memory + w * h * i;
    buffer[i] = memory + w * h * (i + 4);
  }

  for (int j = 0; j < w * h; j++)
    data[0][j] = (v4) { image_data[j].x, image_data[j].y, image_data[j].z, 0.0 };

  memcpy(data[1], data[0], w * h * sizeof(v4));
  memcpy(data[2], data[0], w * h * sizeof(v4));
  memcpy(data[3], data[0], w * h * sizeof(v4));

  #pragma omp parallel for simd
  for (int i = 0; i < 4; i++) {
    blur(data[i], buffer[i], w, h, kernels[i]);
    blur(data[i], buffer[i], w, h, kernels[i]);
    blur(data[i], buffer[i], w, h, kernels[i]);
    blur(data[i], buffer[i], w, h, kernels[i]);
    blur(data[i], buffer[i], w, h, kernels[i]);
  }

  v3* tiny   = difference(data[0], data[1], w, h);
  v3* small  = difference(data[1], data[2], w, h);
  v3* medium = difference(data[2], data[3], w, h);

  write_image(count > 1 ? "data/flower_tiny.ppm"   : 0, tiny,   w, h);
  write_image(count > 1 ? "data/flower_small.ppm"  : 0, small,  w, h);
  write_image(count > 1 ? "data/flower_medium.ppm" : 0, medium, w, h);

  if (count > 1) {
    check("data/flower_tiny.ppm",   "data/flower_tiny_correct.ppm");
    check("data/flower_small.ppm",  "data/flower_small_correct.ppm");
    check("data/flower_medium.ppm", "data/flower_medium_correct.ppm");
  }
}