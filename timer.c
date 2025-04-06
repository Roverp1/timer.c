#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

int parse_time(const char *arg);
void run_timer(int total_seconds);
void print_intro();
void restore_terminal();
void enable_raw_input();
void enable_non_blocking_input();

int main(int argc, char *argv[]) {
  int total_seconds;

  if (argc < 2) {
    printf("Not enough arguments: usage of %s\n", argv[0]);
    return 1;
  }

  total_seconds = parse_time(argv[1]);

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

  return 0;
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
  printf("Timer started (press 'p' to pause)\n\n");
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
    }

    if (total_seconds == 0)
      system("play -n synth 0.1 sine 1000 vol 0.5 > /dev/null 2>&1");

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

void restore_terminal() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
  printf("\033[?25h");
}
