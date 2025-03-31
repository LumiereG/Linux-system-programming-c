#define _XOPEN_SOURCE 500

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define MAX_BUFFER_SIZE 500
#define REQUIREMENTS_FILE "requirements"

// Function to change the current working directory and return the previous directory
char* change_directory(char *dir)
{
    char* previous_directory = (char*)malloc(MAX_BUFFER_SIZE);
    if (getcwd(previous_directory, MAX_BUFFER_SIZE) == NULL)
        ERR("getcwd");
    if (chdir(dir))
        ERR("chdir");
    return previous_directory;

}

// Function to create a new repository directory and initialize a requirements file
void create_repository(char* dir)
{
    FILE* output;
    if(mkdir(dir, 0755) == -1)
    {
        if(errno == EEXIST)
            ERR("the directory already exists");
        ERR("mkdir");
        }
    char* previous_directory = change_directory(dir);
    if((output = fopen(REQUIREMENTS_FILE, "w")) == NULL) 
        ERR("open");
    fclose(output);
    chdir(previous_directory);
}

// Function to check if a package is already listed in the requirements file
int check_package_exists(char* buffer, char* package_name)
{
    int name_length = strchr(buffer, ' ') - buffer;
    char* extracted_name = (char*)malloc(name_length + 1);
    strncpy(extracted_name, buffer, name_length);
    extracted_name[name_length] = '\0';
    if (strcmp(package_name, extracted_name) == 0) return 0;
    return 1;
}

// Function to add a new package entry to the requirements file
void add_package_entry(char* package_name, char* package_version)
{ 
    FILE* output;
    if((output = fopen(REQUIREMENTS_FILE, "a")) == NULL) 
        ERR("open");
    fprintf(output, "%s %s\n", package_name, package_version);
    fclose(output);

    // Creating a new file for the package
    int output_fd;
    if((output_fd = open(package_name, O_WRONLY | O_CREAT | O_TRUNC, 0444)) == -1) 
        ERR("open");
    for (int i = 0; i < rand() % MAX_BUFFER_SIZE; i++)
    {
        char *buf = malloc(sizeof(char));
        buf[0] = rand() % 128;
        write(output_fd, buf, 1);
        free(buf);
    }
    close(output_fd);
}

// Function to add a new package to a repository
void add_new_package(char* input_str, char* repository_dir)
{
    int name_length = strchr(input_str, '=') - input_str;
    char* package_name = (char*)malloc(name_length + 1);
    strncpy(package_name, input_str, name_length);
    package_name[name_length] = '\0';
    char* package_version = strrchr(input_str, '=') + 1; 

    char* previous_directory = change_directory(repository_dir);
    FILE* requirements_file;
    if((requirements_file = fopen(REQUIREMENTS_FILE, "r")) == NULL) 
        ERR("open");

    char buffer[MAX_BUFFER_SIZE];
    int exists = 0;
    while(fgets(buffer, MAX_BUFFER_SIZE, requirements_file) != NULL)
    {
        if(check_package_exists(buffer, package_name) == 0)
        { 
            exists = 1;
            break;
        }
    }
    fclose(requirements_file);
    if(exists == 0)
    {
        add_package_entry(package_name, package_version);
    }
    else ERR("Package already exists");

    chdir(previous_directory);
}

// Function to remove a package from the repository
void remove_package(char *repository_dir, char* package_name)
{
    char* previous_directory = change_directory(repository_dir);
    FILE *file;
    if((file = fopen(REQUIREMENTS_FILE, "r")) == NULL) 
        ERR("open");

    FILE* tmp;
    if((tmp = fopen("tmp", "w")) == NULL) 
        ERR("open");

    int exists = 0;
    char* buffer = malloc(MAX_BUFFER_SIZE);

    while(fgets(buffer, MAX_BUFFER_SIZE, file) != NULL)
    {
        if(check_package_exists(buffer, package_name) == 0)
        { 
            exists = 1;
        }
        else fputs(buffer, tmp);
    }
    fclose(file);
    fclose(tmp);

    if(exists == 1)
    {
        if(unlink(REQUIREMENTS_FILE) == -1) ERR("unlink");
        if(unlink(package_name) == -1) ERR("unlink");
        rename("tmp", REQUIREMENTS_FILE);
    }
    else if(unlink("tmp") == -1) ERR("unlink");
    
    free(buffer);
    chdir(previous_directory);
}

int main(int argc, char** argv)
{
    char** repository_dirs = NULL;
    int dir_count = 0;
    int create_new = 0;
    char* input_package = NULL;
    char* remove_package_name = NULL;

    int option = 0;
    while ((option = getopt(argc, argv, "c::v:i:r:")) != -1)
        switch (option)
        {
            case 'c':
                create_new = 1;
                break;
            case 'v':
                repository_dirs = realloc(repository_dirs, sizeof(char*) * (dir_count + 1));
                repository_dirs[dir_count] = optarg;
                dir_count++;
                break;
            case 'i':
                input_package = optarg;
                break;
            case 'r':
                remove_package_name = optarg;
                break;
            case '?':
            default:
                ERR("Invalid argument");
        }

    if (create_new) 
    {
        if(dir_count != 1) ERR("Incorrect number of arguments");
        else create_repository(repository_dirs[0]);
    }

    if(input_package != NULL)
    {
        srand(time(NULL));
        for (int i = 0; i < dir_count; i++)
            add_new_package(input_package, repository_dirs[i]);
    }

    if(remove_package_name != NULL)
    {
        for (int i = 0; i < dir_count; i++)
            remove_package(repository_dirs[i], remove_package_name);
    }
    
    free(repository_dirs);
    return EXIT_SUCCESS;
}
