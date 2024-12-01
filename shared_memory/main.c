#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
    char str[128];
    int number;
} test_data;

#define SHM_NAME "/test_shm"
#define SHM_SIZE sizeof(test_data)

int main() {
    int shm_fd;
    test_data* shared_data;

    // Создаем общую память
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) 
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Устанавливаем размер общей памяти
    if (ftruncate(shm_fd, SHM_SIZE) == -1) 
    {
        perror("ftruncate");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    // Отображаем память в адресное пространство
    shared_data = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) 
    {
        perror("mmap");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    pid_t cpid = fork();
    if (cpid == -1) 
    {
        perror("fork");
        munmap(shared_data, SHM_SIZE);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    if (cpid > 0) 
    {
        // Родительский процесс
        printf("Parent: Writing data to shared memory...\n");
        strncpy(shared_data->str, "Hello from parent", sizeof(shared_data->str) - 1);
        shared_data->str[sizeof(shared_data->str) - 1] = '\0'; // На всякий случай
        shared_data->number = 42;

        // Ждем завершения дочернего процесса
        wait(NULL);

        // Читаем измененные данные из shared memory
        printf("Parent: Data after child process:\n");
        printf("  String: %s\n", shared_data->str);
        printf("  Number: %d\n", shared_data->number);

        // Очистка
        munmap(shared_data, SHM_SIZE);
        shm_unlink(SHM_NAME);

    } 
    else 
    {
        // Дочерний процесс
        printf("Child: Reading data from shared memory...\n");
        printf("  String: %s\n", shared_data->str);
        printf("  Number: %d\n", shared_data->number);

        // Изменяем данные в shared memory
        printf("Child: Modifying data in shared memory...\n");
        strncpy(shared_data->str, "Hello from child", sizeof(shared_data->str) - 1);
        shared_data->str[sizeof(shared_data->str) - 1] = '\0'; // На всякий случай
        shared_data->number = 100;

        munmap(shared_data, SHM_SIZE);
        exit(EXIT_SUCCESS);
    }

    return 0;
}
