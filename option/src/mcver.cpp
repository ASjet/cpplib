#include <iostream>
#include <string>
#include <vector>
#include <ncurses.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <regex>
#include <atomic>
#include <algorithm>
#include "option.h"
////////////////////////////////////////////////////////////////////////////////
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace fs = std::filesystem;

const string SPLIT(".");
const string libname("libraries");
const std::regex libptn("libraries\\.(.*)");
const string cur_version("cur");
std::atomic<bool> loaded(false);
////////////////////////////////////////////////////////////////////////////////
struct Lib {
  Lib(const fs::path& _name, const string& _version) : name(_name), version(_version) {}
  fs::path name;
  string version;
};

class Option_Lib : public Option<Lib> {
  public:
    Option_Lib(const Lib& _Value, int _MaxLength) : Option(_Value, _MaxLength) {}
    virtual void print(WINDOW* win, int y, int x, bool highlight) const{
      if(highlight)
        attron(COLOR_PAIR(1));
      else
        attron(COLOR_PAIR(0));
      mvwprintw(win, y, 0, "                   ");
      mvwprintw(win, y, x, "%s", value.version.c_str());
      attroff(COLOR_PAIR(1));
    }
    Option_Lib& operator=(const Lib& rhs) {
      value = rhs;
      return *this;
    }
    bool operator<(const Option_Lib& rhs) const {
      return value.name < rhs.value.name;
    }
    void addsurfix() {
      fs::path newname = value.name.parent_path() / (libname + SPLIT + value.version);
      fs::rename(value.name, newname);
      value.name = newname;
    }
    void trimsurfix() {
      fs::path newname = value.name.parent_path() / libname;
      fs::rename(value.name, newname);
      value.name = newname;
    }
    string version() const {
      return value.version;
    }
};

const Lib empty("unknown", "unknown");
Option_Lib current(empty, 10);

class Selector_Lib : public Selector<Option_Lib> {
  public:
    virtual void printAll(WINDOW* win, int highlight) const {
      int len = size();
      wclear(win);
      mvwprintw(win, 0, 0, "Current library: %s", current.version().c_str());
      mvwprintw(win, 1, 0, "Select library: ");
      for(int i = 0; i < len; ++i)
        this->operator[](i).print(win, 2+i, 0, i == highlight);
      wmove(win, 0, 0);
    }
};
////////////////////////////////////////////////////////////////////////////////
Selector_Lib libs;

void readDir(const fs::path& path) {
  fs::path curlib = path / libname;
  if(!fs::exists(curlib)) {
    std::cerr << "No libraries found. Please cheak path: " << curlib << endl;
    endwin();
    exit(-1);
  }

  string version;
  std::ifstream (path / cur_version) >> version;
  current = Lib(curlib, version);

  for(auto& e : fs::directory_iterator(path)) {
    string name = e.path().filename().string();
    std::smatch desc;
    if(std::regex_search(name, desc, libptn)) {
      libs.emplace_back(Lib(name, desc.str(1)), 10);
    }
  }
  loaded = true;
}
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char ** argv)
{
  fs::path path;
  if(argc == 1)
    path = "./";
  else
    path = argv[1];

  fs::path cur = path / cur_version;
  std::thread t(readDir, path);
  t.detach();

  WINDOW* win = initscr();
  noecho();
  start_color();
  init_pair(0, COLOR_WHITE, COLOR_BLACK);
  init_pair(1, COLOR_BLACK, COLOR_WHITE);

  // Wait for loading
  while(!loaded)
    std::this_thread::yield();

  while(true) {
    int index = libs.select(win, nullptr);

    current.addsurfix();
    libs.emplace_back(current);

    current = libs[index];
    libs.erase(libs.begin() + index);

    current.trimsurfix();
    std::ofstream f(cur);
    f << current.version();
  }
  return 0;
}