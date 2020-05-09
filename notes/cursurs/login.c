#define _GNU_SOURCE

#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define PASSWORD_LEN 12
#define USERNAME_LEN 20

bool isempty(char* str) {
  if (str == NULL) return false;
  while (*str && *str == ' ') {
    str++;
  }
  return !*str;
}

void display_box() {
  move(0, 0);
  for (int i = 0; i < 80; i++) {
    printw("*");
  }
  for (int i = 1; i < 16; i++) {
    move(i, 0);
    printw("*");
    move(i, 79);
    printw("*");
  }
  move(16, 0);
  for (int i = 0; i < 80; i++) {
    printw("*");
  }
}

int main() {
  initscr();
  display_box();
  move(5, 10);
  printw("*** LOGIN SCREEN ***");
  char name[USERNAME_LEN + 1];
  do {
    move(7, 10);
    printw("Username: ");
    memset(name, '\0', sizeof(name));
    getnstr(name, USERNAME_LEN);
  } while (isempty(name));
  move(9, 10);
  printw("Password: ");
  char password[PASSWORD_LEN + 1];
  memset(password, '\0', sizeof(password));
  refresh();

  // disables line buffering and erase / kill character processing
  cbreak();
  // disables chars typed by user being echoed as they are being typed
  noecho();
  int i = 0;
  while (true) {
    char c = getch();
    if (c == '\n') {
      password[i] = '\0';
      break;
    }
    // this is a backspace
    if (c == '\b') {
      if (i > 0) {
        i--;
        move(9, 20 + i);
        // hack
        printw(" ");
        move(9, 20 + i);
      }
    } else {
      if (i < PASSWORD_LEN) {
        move(9, 20 + i);
        addch('*');
        password[i++] = c;
      }
    }
    refresh();
  }
  echo();
  nocbreak();

  move(11, 10);
  const char* actual_password = "password";
  if (strcmp(password, actual_password) == 0) {
    printw("%s, you may proceed.", name);
  } else {
    printw("username / password [%s / %s] is incorrect.", name, password);
  }
  getch();
  endwin();
  return EXIT_SUCCESS;
}
