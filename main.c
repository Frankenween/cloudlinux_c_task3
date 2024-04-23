#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>

int main(int argc, const char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Expected 4 arguments: prog1, prog2, prog3 and file, but got %d args\n", argc);
        return 1;
    }
    // Run first process and check its exit code
    __pid_t pid = fork();
    if (pid == -1) {
        perror("fork for prog1 failed");
        return 1;
    } else if (pid == 0) {
        execlp(argv[1], argv[1], (char *) NULL);
        // Failed to exec
        perror("prog1 exec failed");
        return -1;
    }
    int prog1_status;
    wait(&prog1_status);
    if (WEXITSTATUS(prog1_status) != 0) { // prog1 && ... failed
        return WEXITSTATUS(prog1_status);
    }

    // Now create pipe between prog2 and prog3 and open file for output
    int pipe_fd[2];
    if (pipe(pipe_fd)) {
        perror("could not open pipe");
        return 1;
    }
    // Open file before running prog2
    int output_fd = open(argv[4], O_WRONLY | O_TRUNC | O_CREAT,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (output_fd == -1) {
        perror("failed to open file for writing");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return 1;
    }
    __pid_t pid2 = fork(); // to run prog2
    if (pid2 == -1) {
        perror("fork for prog2 failed");
        close(output_fd);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return 1;
    } else if (pid2 == 0) {
        close(pipe_fd[0]); // close read end
        close(output_fd); // close output file - prog2 does not know about it

        dup2(pipe_fd[1], STDOUT_FILENO); // redirect output to pipe

        close(pipe_fd[1]); // remove one reference and free descriptor
        execlp(argv[2], argv[2], (char *) NULL);

        perror("prog2 exec failed");
        return -1;
    }
    __pid_t pid3 = fork(); // run prog3
    if (pid3 == -1) {
        perror("fork for prog3 failed");
        // close file and pipe
        close(output_fd);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        wait(NULL); // wait until prog2 finishes
        return 1;
    } else if (pid3 == 0) {
        close(pipe_fd[1]);

        // Redirect input
        dup2(pipe_fd[0], STDIN_FILENO);
        dup2(output_fd, STDOUT_FILENO);

        // Remove duplicating descriptors
        close(pipe_fd[0]);
        close(output_fd);

        execlp(argv[3], argv[3], (char *) NULL);

        perror("prog3 exec failed");
        return -1;
    }
    // close pipes so when prog2 finishes, pipe will be closed. also close file
    close(output_fd);
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    int prog2_status;
    int prog3_status;
    waitpid(pid2, &prog2_status, 0);
    waitpid(pid3, &prog3_status, 0);
    if (WEXITSTATUS(prog2_status) != 0) return WEXITSTATUS(prog2_status);
    return WEXITSTATUS(prog3_status);
}
