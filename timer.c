#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int hours = 0;
  int minutes = 0;
  int seconds = 0;

  int num = 0;
  int total_seconds = hours * 3600 + minutes * 60 + seconds;

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
    printf("Provide time that is greater than 0\n");
    return 1;
  }

  /*printf("Selected time is: %s\n", argv[1]);*/
  /*printf("Parsed time is: \nhours = %d \nminutes = %d \nseconds = %d\n",
   * hours,*/
  /*       minutes, seconds);*/
  /*printf("Total amount of seconds = %d\n", total_seconds);*/

  printf("Time left:\n");
  while (total_seconds >= 0) {
    int h = total_seconds / 3600;
    int m = (total_seconds % 3600) / 60;
    int s = ((total_seconds % 3600) % 60);

    printf("\r%02d:%02d:%02d", h, m, s);
    fflush(stdout);

    if (total_seconds == 0)
      system("play -n synth 0.1 sine 1000 vol 0.5 > /dev/null 2>&1");

    sleep(1);
    total_seconds--;
  }

  /*system("play -n synth 0.1 sine 1000 vol 0.5 > /dev/null 2>&1");*/
  /*sleep(1);*/
  printf("\n\nTime's up 󱫑\n");

  return 0;
}
