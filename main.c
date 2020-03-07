#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void freads(void * ptr, size_t size, size_t nitems, FILE *file) {
    size_t read = fread(ptr, size, nitems, file);
    if (read < nitems) {
        if (feof(file)) {
            printf("Unexpected eof. Probably wrong format.");
            exit(1);
        }
        if (ferror(file)) {
            printf("Unexpected error(%d) while reading from file", errno);
            exit(1);
        }
    }
}

void fwrites(void * ptr, size_t size, size_t nitems, FILE *file) {
    size_t write = fwrite(ptr, size, nitems, file);
    if (write < nitems) {
        if (feof(file)) {
            printf("Unexpected eof. Probably wrong format.");
            exit(1);
        }
        if (ferror(file)) {
            printf("Unexpected error(%d) while reading from file", errno);
            exit(1);
        }
    }
}

void read_int(FILE *file, unsigned *src) {
    unsigned char a = fgetc(file);
    while (a != ' ' && a != '\n') {
        if (!(a >= '0' && a <= '9')) {
            printf("Wrong file format");
            exit(1);
        }
        *src *= 10;
        *src += a - '0';
        a = fgetc(file);
    }
}

typedef struct {
    unsigned char r, g, b;
} RGB;

void rotate_left(void **data, unsigned width, unsigned height, unsigned size, FILE  *output) {
    fprintf(output, "%d %d\n", height, width);
    fprintf(output, "%d\n", 255);

    for (unsigned i = 0; i < width; ++i) {
        for (unsigned j = 0; j < height; ++j) {
            for (unsigned k = 0; k < size; ++k) {
                fwrites(data[j] + k + i * size, 1, 1, output);
            }
        }
    }
}

void rotate_right(void **data, unsigned width, unsigned height, unsigned size, FILE  *output) {
    fprintf(output, "%d %d\n", height, width);
    fprintf(output, "%d\n", 255);

    for (unsigned i = 0; i < width; ++i) {
        for (unsigned j = 1; j <= height; ++j) {
            for (unsigned k = 0; k < size; ++k) {
                fwrites(data[height - j] + k + i * size, 1, 1, output);
            }
        }
    }
}

void apply_mirror_effect_vertical(void **data, unsigned width, unsigned height, unsigned size, FILE  *output) {
    fprintf(output, "%d %d\n", width, height);
    fprintf(output, "%d\n", 255);

    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j) {
            for (unsigned k = 0; k < size; ++k) {
                fwrites(data[i] + k + (width - j - 1) * size, 1, 1, output);
            }
        }
    }
}

void apply_mirror_effect_horizontal(void **data, unsigned width, unsigned height, unsigned size, FILE  *output) {
    fprintf(output, "%d %d\n", width, height);
    fprintf(output, "%d\n", 255);

    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j) {
            for (unsigned k = 0; k < size; ++k) {
                fwrites(data[height - i - 1] + k + j * size, 1, 1, output);
            }
        }
    }
}

void inverse(void **data, unsigned width, unsigned height, unsigned size, FILE  *output) {
    printf("%ld\n", (long)data[0]);
    fprintf(output, "%d %d\n", width, height);
    fprintf(output, "%d\n", 255);

    printf("%d\n", size);

    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j) {
            for (unsigned k = 0; k < size; ++k) {
                unsigned char a = *((char *) (data[i] + k + j * size));
                a ^= 255;
                fwrites(&a, 1, 1, output);
            }
        }
    }
}

int main(int argc, char **args) {
    if (argc < 4) {
        printf("pnm <input> <output> <command>");
        return 1;
    }
    FILE *input = fopen(args[1], "rb");
    if (input == NULL) {
        printf("Error %d while opening file %s", errno, args[1]);
        return 1;
    }
    unsigned char type[3];
    freads(type, 3, 1, input);
    if (type[0] != 'P') {
        printf("Unrecognized file format");
        return 1;
    }
    if (type[2] != '\n') {
        printf("Unrecognized file format\n");
        return 1;
    }
    int colorized;
    if (type[1] == '5') {
        colorized = 0;
    } else if (type[1] == '6') {
        colorized = 1;
    } else {
        printf("Unrecognized color format");
        return 1;
    }
    unsigned width = 0, height = 0;
    read_int(input, &width);
    read_int(input, &height);

    void **matrix;
    unsigned size;
    if (colorized) {
        matrix = malloc(sizeof(RGB *) * height);
        size = 3;
    } else {
        matrix = malloc(height * sizeof(void *));
        size = 1;
    }
    unsigned depth;
    read_int(input, &depth);
    printf("depth: %d\nwidth: %d\nheight: %d\nsize: %d\n", depth, width, height, size);
    for (unsigned i = 0; i < height; ++i) {
        matrix[i] = malloc(width * size);
        if (matrix[i] == NULL)
            return 1;
        freads(matrix[i], size, width, input);
    }


    FILE *output;
    if (strcmp(args[3], "rr") == 0) {
        output = fopen(args[2], "wb");
        if (output == NULL) {
            printf("Error %d while opening file %s", errno, args[2]);
            return 1;
        }

        fwrites(type, 1, 3, output);

        rotate_right(matrix, width, height, size, output);
    } else if (strcmp(args[3], "rl") == 0) {
        output = fopen(args[2], "wb");
        if (output == NULL) {
            printf("Error %d while opening file %s", errno, args[2]);
            return 1;
        }

        fwrites(type, 1, 3, output);

        rotate_left(matrix, width, height, size, output);
    } else if (strcmp(args[3], "mv") == 0) {
        output = fopen(args[2], "wb");
        if (output == NULL) {
            printf("Error %d while opening file %s", errno, args[2]);
            return 1;
        }

        fwrites(type, 1, 3, output);

        apply_mirror_effect_vertical(matrix, width, height, size, output);
    } else if (strcmp(args[3], "mh") == 0) {
        output = fopen(args[2], "wb");
        if (output == NULL) {
            printf("Error %d while opening file %s", errno, args[2]);
            return 1;
        }

        fwrites(type, 1, 3, output);

        apply_mirror_effect_horizontal(matrix, width, height, size, output);
    }  else if (strcmp(args[3], "i") == 0) {
        output = fopen(args[2], "wb");
        if (output == NULL) {
            printf("Error %d while opening file %s", errno, args[2]);
            return 1;
        }

        fwrites(type, 1, 3, output);

        inverse(matrix, width, height, size, output);
    } else {
        printf("Unrecognized mode %s", args[3]);
        return 1;
    }



    for (unsigned j = 0; j < height; ++j) {
        free(matrix[j]);
    }
    free(matrix);

    fclose(output);
    fclose(input);

    return 0;
}
