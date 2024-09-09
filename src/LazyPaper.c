#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    char *splash_prefix = "false";
    char *ipc_prefix = "off";
    const char *obtain_monitor = "hyprctl monitors | grep -oP '(?<=Monitor )\\S+'";
    const char *obtain_username = "whoami";
    const char *kill_process = "pkill hyprpaper";
    const char *reload_process = "nohup hyprpaper </dev/null >/dev/null 2>&1 &";
    char monitor_name[10] = {0};
    char username[20] = {0};
    char path[256] = {0};

    FILE *pipe_1 = popen(obtain_monitor, "r");
    if (pipe_1 == NULL) {
        perror("Failed to retrieve monitor.");
        return EXIT_FAILURE;
    }

    FILE *pipe_2 = popen(obtain_username, "r");
    if (pipe_2 == NULL) {
        perror("Failed to retrieve username.");
        pclose(pipe_1);
        return EXIT_FAILURE;
    }

    if (fgets(monitor_name, sizeof(monitor_name), pipe_1) != NULL) {
        monitor_name[strcspn(monitor_name, "\n")] = '\0';
    } else {
        perror("No monitor found.");
        pclose(pipe_1);
        pclose(pipe_2);
        return EXIT_FAILURE;
    }

    if (fgets(username, sizeof(username), pipe_2) != NULL) {
        username[strcspn(username, "\n")] = '\0';
    } else {
        perror("No user found.");
        pclose(pipe_1);
        pclose(pipe_2);
        return EXIT_FAILURE;
    }

    pclose(pipe_1);
    pclose(pipe_2);

    snprintf(path, sizeof(path), "/home/%s/.config/hypr/hyprpaper.conf", username);

    FILE *config = fopen(path, "w");
    if (config == NULL) {
        perror("Failed to create hyprpaper.conf");
        return EXIT_FAILURE;
    }

    if (argc < 2) {
        printf("You should drag the file into the terminal.\n");
        fclose(config);
        return EXIT_FAILURE;
    }

    int total_length = 0;
    for (size_t i = 1; i < argc; ++i) {
        total_length += strlen(argv[i]) + 1;
    }

    char *directory = calloc(total_length + 1, sizeof(char));
    if (directory == NULL) {
        perror("Failed to allocate memory");
        fclose(config);
        return EXIT_FAILURE;
    }

    for (size_t i = 1; i < argc; ++i) {
        strcat(directory, argv[i]);
        if (i < argc - 1) {
            strcat(directory, " ");
        }
    }

    fprintf(config, "preload = %s\n", directory);
    fprintf(config, "wallpaper = %s,%s\n", monitor_name, directory);
    fprintf(config, "splash = %s\n", splash_prefix);
    fprintf(config, "ipc = %s\n", ipc_prefix);

    fclose(config);
    free(directory);

    FILE *pipe_3 = popen(kill_process, "r");
    if (pipe_3 == NULL) {
        perror("Failed to kill hyprpaper process.");
        return EXIT_FAILURE;
    }
    pclose(pipe_3);

    FILE *pipe_4 = popen(reload_process, "r");
    if (pipe_4 == NULL) {
        perror("Failed to reload hyprpaper process.");
        return EXIT_FAILURE;
    }
    pclose(pipe_4);

    return EXIT_SUCCESS;
}
