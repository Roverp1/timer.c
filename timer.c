#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void enable_raw_input();

void restore_terminal();

int main(int argc, char *argv[]) {
  int hours = 0;
  int minutes = 0;
  int seconds = 0;

  int num = 0;
  int total_seconds = hours * 3600 + minutes * 60 + seconds;

  int stdin_flags = fcntl(STDIN_FILENO, F_GETFL);
  int non_blocking_stdin_flags = stdin_flags | O_NONBLOCK;

  bool pause_flag = false;

  if (argc < 2) {
    printf("Not enough arguments: usage of %s\n", argv[0]);
    return 1;
  }

  for (int i = 0; argv[1][i] != '\0'; i++) {
    char c = argv[1][i];

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
      return 1;
    }
  }

  total_seconds = hours * 3600 + minutes * 60 + seconds;

  if (total_seconds <= 0) {
    printf("Provide time that is greater than 0\nIn format: 3h2m1s");
    return 1;
  }

  fcntl(STDIN_FILENO, F_SETFL, non_blocking_stdin_flags);
  enable_raw_input();
  atexit(restore_terminal);

  printf("Timer started (press 'p' to pause)\n\n");
  printf("Time left:\n");
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

  printf("\n\nTime's up 󱫑\n");

  return 0;
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

void restore_terminal() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
  printf("\033[?25h");
}
