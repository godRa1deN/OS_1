#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    uid_t pid = -1;                                         //id процессов
    int first_pipe, second_pipe;                   
    char first_name[] = "first.fifo";
    char second_name[] = "second.fifo";
    int input, output, size;
    int answer[2];
    char s[6000];                                           
    if (argc != 3) {                                        //проверка на кол-во аргументов командной строки
        printf("Wrong number of arguments in command line\n");
        exit(-1);
    }                                     
    (void)umask(0);
    mknod(first_name, S_IFIFO | 0666, 0);
    pid = fork();                                         //создание процесса для обработки данных
    if (pid) {                                            
        printf("Read input-file\n");                        //здесь находимся в процессе input
        if ((input = open(argv[1], O_RDONLY, 0666)) < 0) {  //открываем файл для чтения(первый аргумент командной строки)
            printf("Can\'t open input file\n");
            exit(-1);
        }
        size = read(input, s, sizeof(s));                   //считывание текста
        if (size < 0) {
            printf("Can\'t read string from file\n");
            exit(-1);
        }
        s[size] = '\0';                                     //добавляем терминирующий 0
        if ((first_pipe = open(first_name, O_WRONLY)) < 0) {
            printf("Can\'t open first pipe\n");
            exit(-1);
        }
        size = write(first_pipe, s, sizeof(s));
        if (size < 0) {
            printf("Can\'t write to first pipe\n");
            exit(-1);
        }
        if (close(input) < 0) {
            printf("Can\'t close input file\n");
            exit(-1);
        }
        if (close(first_pipe) < 0) {
            printf("input: Can\'t close first pipe\n");
            exit(-1);
        }
        printf("Read process end\n");
    } else {
        if (pid == -1) {                                                  //сюда попадаем если находимся в процессе обработки данных
            printf("Can\'t create new process\n");
            exit(-1);
        }
        mknod(second_name, S_IFIFO | 0666, 0);                            //создание канала обработка данных -> вывод
        pid = fork();                                                     //создание процесса для вывода текста                                                   
        if (pid) {
            printf("Data processing process\n");                            //сюда попадаем если находимся в процессе обработки данных
            if ((first_pipe = open(first_name, O_RDONLY)) < 0) {            //считывание текста из канала
                printf("Can't open first pipe\n");
                exit(-1);
            }    
            size = read(first_pipe, s, sizeof(s));                   
            if (size < 0) {
                printf("Can\'t read string from first pipe\n");
                exit(-1);
            }
            int i = 0;                                                      //обработка данных
            answer[0] = 0;
            answer[1] = 0;
            while (i < 6000 && s[i] != '\0') {
                if (s[i] >= 'a' && s[i] <= 'z') {
                    answer[0]++;
                }
                if (s[i] >= 'A' && s[i] <= 'Z') {
                    answer[1]++;
                }
                i++;
            }
            if ((second_pipe = open(second_name, O_WRONLY)) < 0) {          //записываем результаты вычислений в канал 2
                printf("Can\'t open second pipe\n");
                exit(-1);
            }
            size = write(second_pipe, &answer, 2 * sizeof(int));        
            if (size != 2 * sizeof(int)) {
                printf("Can\'t write all string to second pipe\n");               
                exit(-1);
            }
            if (size < 0) {
                printf("Can\'t write string to second pipe\n");
                exit(-1);
            }
            if (close(first_pipe) < 0) {                 
                printf("parent: Can\'t close frist pipe\n");      
                exit(-1);
            }
            if (close(second_pipe) < 0) {
                printf("parent: Can\'t close second pipe\n");
                exit(-1);
            }
            printf("Data processing process end\n");
        } else {
            if (pid == -1) {
                printf("Can\'t create new process\n");
                exit(-1);
            }
            printf("output process\n");
            if ((second_pipe = open(second_name, O_RDONLY)) < 0) {
                printf("Can\'t open second pipe\n");
                exit(-1);
            }
            size = read(second_pipe, &answer, 2 * sizeof(int));
            if (size < 0) {
                printf("Can\'t read string from pipe\n");
                exit(-1);
            }
            if ((output = open(argv[2], O_WRONLY, 0666)) < 0) {                  //открываем файл для записи результата
                printf("Can\'t open output file\n");
                exit(-1);
            }
            char s_output[100];                                                  //создаем строку в которой будет храниться ответ
            sprintf(s_output, "Ответ: Строчных букв: %d Заглавных букв: %d\n", answer[0], answer[1]);
            size = write(output, s_output, strlen(s_output));
            if (size < 0) {
                printf("Can\'t write string to output\n");
                exit(-1);
            }
            if (close(output) < 0) {
                printf("Can\'t close output file\n");
            }
            if (close(second_pipe) < 0) {
                printf("child: Can\'t close second pipe\n");
                exit(-1);
            }
            printf("output process end\n");
        }
    }
    return 0;
}