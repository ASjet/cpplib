#ifndef OPTION_H
#define OPTION_H

#include <ncurses.h>
#include <vector>

template<typename T>
class Option {
  public:
    Option(const T& _Value, int _MaxLength): value(_Value), length(_MaxLength) {}
    virtual void print(WINDOW* win, int y, int x, bool highlight) const = 0;
    Option& operator=(const T& rhs) {
      value = rhs;
      return *this;
    }
  protected:
    T value;
    int length;
};

template<typename T>
class Selector: public std::vector<T> {
  public:
    virtual void printAll(WINDOW* win, int highlight) const = 0;
    int select(WINDOW* win, bool(*comp)(const T& lhs, const T& rhs));
};

template <typename T>
int Selector<T>::select(WINDOW* win, bool(*comp)(const T& lhs, const T& rhs)) {
  const int cnt = this->size();
  int key = 0, index = 0;
  bool chose = false;

  if(comp == nullptr)
    std::sort(this->begin(), this->end());
  else
    std::sort(this->begin(), this->end(), comp);

  do {
    switch (key) {
      case 'k': // UP
      case 'l': // RIGHT
        index = (index + cnt - 1) % cnt;
        break;
      case 'j': // DOWN
      case 'h': // LEFT
        index = (index + 1) % cnt;
        break;
      case '\n': // ENTER
        chose = true;
        break;
      default:
        break;
    }
    if(chose)
      break;
    printAll(win, index);
  } while ((key = getch()) != 'q');

  if(key == 'q') {
    endwin();
    exit(0);
  }
  return index;
}

#endif