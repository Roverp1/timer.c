#include "sys/wait.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

typedef struct {
  char *time;
  bool alarm_mode;
  bool silet_mode;
} UserConfig;

void print_help_msg();
char **check_dependencies();
int parse_time(const char *arg);
void enable_raw_input();
void enable_non_blocking_input();
void print_intro();
void run_timer(int total_seconds);
void run_alarm(bool alarm_mode);
void restore_terminal();
void print_unavailable_dependecies(char **unavailable_dependecies);
void free_unavailable_dependecies(char **unavailable_dependecies);

char *dependencies[] = {"play", /*"test1", "test2",*/ NULL};

int main(int argc, char *argv[]) {
  UserConfig user_config = {
      .time = NULL,
      .alarm_mode = false,
      .silet_mode = false,
  };
  int total_seconds;
  char **unavailable_dependecies = check_dependencies();

  if (argc <= 1) {
    printf("Not enough arguments: use `ctimer -h` to veiw help message\n");
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
      print_help_msg();
    else if (strcmp(argv[i], "--alarm") == 0 || strcmp(argv[i], "-a") == 0)
      user_config.alarm_mode = true;
    else if (strcmp(argv[i], "--silet") == 0 || strcmp(argv[i], "-s") == 0)
      user_config.silet_mode = true;

    else {
      if (user_config.time == NULL)
        user_config.time = argv[i];
      else {
        fprintf(stderr, "Use `ctimer -h` menu for available arguments\n");
        exit(1);
      }
    }
  }

  total_seconds = parse_time(user_config.time);

  if (total_seconds <= 0) {
    printf("Provide time that is greater than 0\nIn format: 3h2m1s\n");
    return 1;
  }

  enable_non_blocking_input();
  enable_raw_input();
  atexit(restore_terminal);

  print_intro();
  run_timer(total_seconds);
  printf("\n\nTime's up 󱫑\n");
  if (!user_config.silet_mode)
    run_alarm(user_config.alarm_mode);

  free_unavailable_dependecies(unavailable_dependecies);

  return 0;
}

void print_help_msg() {
  printf("Usage: timer [duration]\n");
  printf("Example: timer 1h2m30s\n\n");

  printf("Optins:\n");
  printf("--alarm or -a makes noise until you stop it\n");
  printf("--silet or -s makes no noise\n");
  printf("\n");

  printf("Controls:\n");
  printf("  p  Pause/resume\n");
  printf("  q  Quit early\n");

  exit(0);
}

// TODO: FREE THE FUCKIN' SPACE
char **check_dependencies() {
  char **unavailable_dependecies = NULL;
  int unavailable_dependecies_length = 0;

  for (int i = 0; dependencies[i] != NULL; i++) {
    char check_dep_command[256];
    int dependency_status;

    snprintf(check_dep_command, sizeof(check_dep_command),
             "command -v %s > /dev/null 2>&1", dependencies[i]);

    dependency_status = system(check_dep_command);

    if (WIFEXITED(dependency_status) && WEXITSTATUS(dependency_status) != 0) {
      unavailable_dependecies =
          realloc(unavailable_dependecies,
                  (unavailable_dependecies_length + 1) * sizeof(char *));

      unavailable_dependecies[unavailable_dependecies_length] =
          strdup(dependencies[i]);

      unavailable_dependecies_length++;
    }
  }
  unavailable_dependecies =
      realloc(unavailable_dependecies,
              (unavailable_dependecies_length + 1) * sizeof(char *));

  unavailable_dependecies[unavailable_dependecies_length] = NULL;
  unavailable_dependecies_length++;

  return unavailable_dependecies;
}

int parse_time(const char *arg) {
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  int num = 0;
  int total_seconds;

  for (int i = 0; arg[i] != '\0'; i++) {
    char c = arg[i];

    if (isdigit(c)) {
      num = num * 10 + (c - '0');
    } else if (c == 'h') {
      hours = num;
      num = 0;
    } else if (c == 'm') {
      minutes = num;
      num = 0;
    } else if (c == 's') {
      seconds = num;
      num = 0;
    } else {
      printf("Error while parsing %c\n", c);
      return -1;
    }
  }

  total_seconds = hours * 3600 + minutes * 60 + seconds;

  return total_seconds;
}

void enable_raw_input() {
  struct termios new_termios;

  // get current terminal settings
  tcgetattr(STDIN_FILENO, &orig_termios);
  new_termios = orig_termios;

  // disable line buffering n echo mode
  new_termios.c_lflag &= ~(ICANON | ECHO);

  // set terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
  printf("\033[?25l");
}

void enable_non_blocking_input() {
  int stdin_flags = fcntl(STDIN_FILENO, F_GETFL);
  int non_blocking_stdin_flags = stdin_flags | O_NONBLOCK;

  fcntl(STDIN_FILENO, F_SETFL, non_blocking_stdin_flags);
}

void print_intro() {
  printf("Timer started (press 'p' to pause, 'q' to quit)\n\n");
  printf("Time left:\n");
}

void run_timer(int total_seconds) {
  bool pause_flag = false;

  while (total_seconds >= 0) {
    int h = total_seconds / 3600;
    int m = (total_seconds % 3600) / 60;
    int s = ((total_seconds % 3600) % 60);

    int user_input;
    while ((user_input = getchar()) != EOF) {
      if (user_input == 'p') {
        if (pause_flag == true)
          printf("\033[2K\033[1A");
        else if (pause_flag == false)
          printf("\n");
        fflush(stdout);

        pause_flag = !pause_flag;
        break;
      }

      if (user_input == 'q') {
        printf("\nQuitting early...\n");
        fflush(stdout);
        exit(0);
      }
    }

    if (!pause_flag) {
      printf("\r\033[2K%02d:%02d:%02d", h, m, s);
      total_seconds--;
    } else {
      printf("\033[2K\rPAUSED...");
    }
    fflush(stdout);
    sleep(1);
  }
}

void run_alarm(bool alarm_mode) {
  int user_input;

  if (alarm_mode)
    printf("Press any key to stop alarm...\n");

  do {
    system("play -n synth 0.1 sine 1000 vol 0.5 > /dev/null 2>&1");
    printf("\a");
    sleep(1);
  } while ((user_input = getchar()) == EOF && alarm_mode);
}

void restore_terminal() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
  printf("\033[?25h");
}

void print_unavailable_dependecies(char **unavailable_dependecies) {
  printf("\nList of unavailable_dependecies:\n");
  for (int i = 0; unavailable_dependecies[i] != NULL; i++) {
    printf("- %s\t", unavailable_dependecies[i]);
  }
  printf("\n\n");
}

void free_unavailable_dependecies(char **unavailable_dependecies) {
  for (int i = 0; unavailable_dependecies[i] != NULL; i++) {
    free(unavailable_dependecies[i]);
  }
  free(unavailable_dependecies);
}
