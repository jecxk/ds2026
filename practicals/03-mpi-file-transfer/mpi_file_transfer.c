// practicals/03-mpi-file-transfer/mpi_file_transfer.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG_NAME_LEN  1
#define TAG_NAME      2
#define TAG_CHUNK_LEN 3
#define TAG_CHUNK     4

#define CHUNK_SIZE 4096

void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    MPI_Abort(MPI_COMM_WORLD, 1);
}

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) {
            fprintf(stderr, "This program must be run with 2 processes.\n");
            fprintf(stderr, "Usage: mpirun -np 2 ./mpi_file_transfer <input> <output>\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        /* ----------- SENDER (client) ----------- */
        if (argc < 3) {
            fprintf(stderr, "Usage: mpirun -np 2 ./mpi_file_transfer <input> <output>\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        const char *input_path  = argv[1];
        const char *output_name = argv[2];

        FILE *in = fopen(input_path, "rb");
        if (!in) {
            perror("Cannot open input file");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        int name_len = (int)strlen(output_name);
        MPI_Send(&name_len, 1, MPI_INT, 1, TAG_NAME_LEN, MPI_COMM_WORLD);
        MPI_Send(output_name, name_len, MPI_CHAR, 1, TAG_NAME, MPI_COMM_WORLD);

        unsigned char buffer[CHUNK_SIZE];
        while (1) {
            int n = (int)fread(buffer, 1, CHUNK_SIZE, in);
            if (n < 0) {
                perror("Error reading input file");
                fclose(in);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            MPI_Send(&n, 1, MPI_INT, 1, TAG_CHUNK_LEN, MPI_COMM_WORLD);
            if (n == 0) {
                break;  // EOF
            }
            MPI_Send(buffer, n, MPI_UNSIGNED_CHAR, 1, TAG_CHUNK, MPI_COMM_WORLD);
        }

        printf("[RANK 0] Finished sending file '%s' as '%s'\n",
               input_path, output_name);
        fclose(in);

    } else {
        /* ----------- RECEIVER (server) ----------- */
        int name_len;
        MPI_Recv(&name_len, 1, MPI_INT, 0, TAG_NAME_LEN, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        char *output_name = malloc((size_t)name_len + 1);
        if (!output_name) die("Allocation failed for file name.");

        MPI_Recv(output_name, name_len, MPI_CHAR, 0, TAG_NAME,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        output_name[name_len] = '\0';

        FILE *out = fopen(output_name, "wb");
        if (!out) {
            perror("Cannot open output file");
            free(output_name);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        unsigned char buffer[CHUNK_SIZE];
        while (1) {
            int n;
            MPI_Recv(&n, 1, MPI_INT, 0, TAG_CHUNK_LEN, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            if (n == 0) break;

            MPI_Recv(buffer, n, MPI_UNSIGNED_CHAR, 0, TAG_CHUNK,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            size_t written = fwrite(buffer, 1, (size_t)n, out);
            if (written < (size_t)n) {
                perror("Error writing to output file");
                fclose(out);
                free(output_name);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }

        printf("[RANK 1] Received file and saved as '%s'\n", output_name);
        fclose(out);
        free(output_name);
    }

    MPI_Finalize();
    return 0;
}
