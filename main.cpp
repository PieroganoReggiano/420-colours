#include <iostream>
#include <array>
#include <vector>
#include <chrono>
#include <random>
#include <list>
#include <set>
#include <optional>

#include "fmt/color.h"

#include "SDL2/SDL.h"
#include "GL/glew.h"

#include "glm/vec2.hpp"






constexpr static const char *vertex_shader=
  "#version 120\n"
  "attribute vec2 vert;\n"
  "attribute vec2 uv;\n"
  "varying vec2 uvf;\n"
  "void main(void)\n"
  "{\n"
  "  gl_Position=vec4(vert,0.0,1.0);\n"
  "  uvf=uv;\n"
  "}\n"
;
constexpr static const char *fragment_shader=
  "#version 120\n"
  "varying vec2 uvf;\n"
  "uniform sampler2D texture;\n"
  "void main(void)\n"
  "{\n"
  "  gl_FragColor=texture2D(texture, uvf);\n"
  "}\n"
;

constexpr auto fmt_color = fg(fmt::rgb(255,0,0)) | bg(fmt::rgb(40,0,0));

GLuint vsh;
GLuint fsh;
GLuint shp;

GLint attr_vert;
GLint attr_uv;

GLuint texture;

SDL_GLContext context;
SDL_Window *p_window;

intptr_t ticrate = 60;

bool weird_rainbow = false;


std::mt19937_64 rng(1337 + 420);


void load_shaders() {
  GLint status;

  vsh = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vsh, 1, &vertex_shader, 0);
  glCompileShader(vsh);

  glGetShaderiv(vsh, GL_COMPILE_STATUS, &status);
  if (!status) {
    GLint d = 0;
    glGetShaderiv(vsh, GL_INFO_LOG_LENGTH, &d);
    std::string s(d, ' ');
    glGetShaderInfoLog(vsh, d, &d, s.data());
    auto msg = fmt::format("Vertex shader: {}", s);
    throw std::runtime_error(msg);
  }

  fsh = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fsh, 1, &fragment_shader, 0);
  glCompileShader(fsh);

  glGetShaderiv(fsh, GL_COMPILE_STATUS, &status);
  if(!status) {
    GLint d = 0;
    glGetShaderiv(fsh, GL_INFO_LOG_LENGTH, &d);
    std::string s(d, ' ');
    glGetShaderInfoLog(fsh, d, &d, s.data());
    auto msg = fmt::format("Fragment shader: {}", s);
    throw std::runtime_error(msg);
  }

  shp = glCreateProgram();
  glAttachShader(shp, vsh);
  glAttachShader(shp, fsh);
  glLinkProgram(shp);

  glGetProgramiv(shp, GL_LINK_STATUS, &status);
  if(!status)
  {
    GLint d = 0;
    glGetProgramiv(shp, GL_INFO_LOG_LENGTH, &d);
    std::string s(d, ' ');
    glGetProgramInfoLog(shp, d, &d, s.data());
    auto msg = fmt::format("Program: {}", s);
    throw std::runtime_error(msg);
  }

  attr_vert = glGetAttribLocation(shp, "vert");
  if(attr_vert == -1)
    throw std::runtime_error("vert nie działa");
  attr_uv = glGetAttribLocation(shp, "uv");
  if(attr_uv == -1)
    throw std::runtime_error("uv nie działa");
}



void draw(intptr_t w, intptr_t h, uint32_t *p_pixels) {

  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    w,
    h,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    p_pixels
  );

  glClear(GL_COLOR_BUFFER_BIT);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


  SDL_GL_SwapWindow(p_window);
  glDeleteTextures(1, &texture);
}


constexpr static float verts[] {
  -1.f, -1.f,
   1.f, -1.f,
   1.f,  1.f,
  -1.f,  1.f,
};

constexpr static float texuv[] {
  0.f, 1.f,
  1.f, 1.f,
  1.f, 0.f,
  0.f, 0.f,
};




struct Cell {
  int value;
  int next_value;
  bool wall;

};

struct Less {
  bool operator()(const glm::i64vec2&l, const glm::i64vec2&r) const {
    return l.x < r.x || (l.x == r.x && l.y < r.y);
  }
};

struct Board;


struct LabyrinthGenerator {
  Board *p_board;
  intptr_t x;
  intptr_t y;

  intptr_t w2;
  intptr_t h2;


  std::list<glm::i64vec2> walls;

  decltype(walls)::iterator it;

  intptr_t iters_left;

  bool not_end;

  LabyrinthGenerator(Board* p_bboard);

  bool iteration();

  bool finish();

};

struct Board {
  std::vector<Cell> cells;
  //std::set<glm::i64vec2, Less> special_cells;
  intptr_t w, h;
  std::optional<LabyrinthGenerator> lgen;

  void set_size(intptr_t w, intptr_t h);
  void clear();
  void gen_lab();
  void advance_gen();
  void step();
  void paste_texture(uint32_t*);
  Cell &cell_at(intptr_t x, intptr_t y);
};

inline void Board::set_size(intptr_t ww, intptr_t hh) {
  cells.resize(ww * hh, Cell { true });
  w = ww;
  h = hh;
}

inline void Board::paste_texture(uint32_t* pixels) {
  for (size_t i = 0; i < cells.size(); ++i) {
    uint32_t color = 0;
    if (!cells[i].wall) {
      color = -1;
      int value = cells[i].value;
      if (value >= 0) {
        color = 0xff000000;
        int h,v;
        h = weird_rainbow ? value%60 : value/60;
        v = 255 - ((value % 60) * 255 / 60);
        int r,g,b;
        r = h==0 || h==1 || h==5 ? v : 0;
        g = h==1 || h==2 || h==3 ? v : 0;
        b = h==3 || h==4 || h==5 ? v : 0;

        color |= (r << 16) | (g << 8) | b;
      }
    }
    pixels[i] = color;
  }
}

inline void Board::step() {
  intptr_t x = 0;
  intptr_t y = 0;
  for (;;) {

    if (!cell_at(x, y).wall) {
      //bool is_special = special_cells.count({x,y});
      intptr_t value = cell_at(x, y).value;
      if (value >= 0) {
        cell_at(x, y).next_value = (value + 1) % 360;
      } else {
        for (glm::i64vec2 v : {glm::i64vec2(1,0),glm::i64vec2(0,1),glm::i64vec2(0,-1),glm::i64vec2(-1,0),}) {
          if (!cell_at(x+v.x, y+v.y).wall && cell_at(x+v.x, y+v.y).value >= 0) {
            cell_at(x,y).next_value = std::max(cell_at(x,y).value, cell_at(x+v.x, y+v.y).value);
          }
        }
      }
    }

    ++x;
    if (x >= w) {
      x = 0;
      ++y;
    }
    if (y >= h)
      break;
  }
  x = 0;
  y = 0;
  for (;;) {

    if (!cell_at(x, y).wall) {
      cell_at(x,y).value = cell_at(x,y).next_value;
    }

    ++x;
    if (x >= w) {
      x = 0;
      ++y;
    }
    if (y >= h)
      break;
  }
}

inline Cell &Board::cell_at(intptr_t x, intptr_t y) {
  return cells.at(x + y * h);
}

inline void Board::clear() {
  for (auto& cell : cells) {
    cell.wall = true;
    cell.value = -1;
    cell.next_value = -1;
  }
}

inline void Board::advance_gen() {
  if (lgen.has_value()) {
    bool res = lgen->iteration();
    if (!res) {
      lgen.reset();
    }
  }
}

inline void Board::gen_lab() {
  lgen = LabyrinthGenerator(this);
}

inline LabyrinthGenerator::LabyrinthGenerator(Board* p_bboard) {
  p_board = p_bboard;
  p_board->clear();
  x = (rng() % p_board->w/2) * 2 + 1;
  y = (rng() % p_board->h/2) * 2 + 1;
  p_board->cell_at(x,y).wall = false;
  w2 = p_board->w - !(p_board->w%2),
  h2 = p_board->h - !(p_board->h%2);
  walls.clear();

  if (y > 0)
    walls.push_back({ x, y - 1 });
  if (x > 0)
    walls.push_back({ x - 1, y });
  if (y < h2 - 1)
    walls.push_back({ x, y + 1 });
  if (x < w2 - 1)
    walls.push_back({ x + 1, y });

  it = walls.begin();

  iters_left = 1000000000;

  not_end = true;
};


inline bool LabyrinthGenerator::iteration() {
  if (!not_end)
    return false;
  if (iters_left <= 0)
    return finish();
  {

    intptr_t plus = 50 + (rng() % 150);
    while (plus > 0) {
      ++it;
      if (it == walls.end())
        it = walls.begin();
      if (it == walls.end())
        return finish();
      --plus;
    }
    {
      // Szukamy, czy sąsiaduje z jednym
      intptr_t wallx = it->x;
      intptr_t wally = it->y;
      glm::i64vec2 nigh (-3,-3);
      for (auto v : { glm::i64vec2(1,0), glm::i64vec2(0,1), glm::i64vec2(-1,0), glm::i64vec2(0,-1), }) {
        if (wallx+v.x > 0 && wallx+v.x < w2-1 && wally+v.y > 0 && wally+v.y < h2-1) {
          if (!p_board->cell_at(wallx+v.x, wally+v.y).wall) {
            if (nigh.x > -2) {
              // zła ściana -- szukamy dalej
              goto skip;
            }
            nigh = v;
          }
        }
      }
      if (nigh.x <= -2) {
        fmt::print(stderr, fmt_color, "jakaś samotna ta ściana\n");
        return false;
      }
      p_board->cell_at(wallx, wally).wall = false;
      glm::i64vec2 scell (wallx-nigh.x, wally-nigh.y);
      if (scell.x < 1 || scell.x > w2-1 || scell.y < 1 || scell.y > h2-1) {
        fmt::print(stderr, fmt_color, "korytarz tam gdzie nie można\n");
        return false;
      }
      p_board->cell_at(scell.x, scell.y).wall = false;
      {
        glm::i64vec2 scell (wallx-nigh.x, wally-nigh.y);
        // Szukamy ścian wokół
        if (scell.y > 1 && scell.x > 0 && scell.x < w2-2 && p_board->cell_at(scell.x, scell.y - 1).wall)
          walls.push_back({ scell.x, scell.y - 1 });
        if (scell.x > 1 && scell.y > 0 && scell.y < h2-2 && p_board->cell_at(scell.x - 1, scell.y).wall)
          walls.push_back({ scell.x - 1, scell.y });
        if (scell.y < h2 - 2 && scell.x > 0 && scell.x < w2-2 && p_board->cell_at(scell.x, scell.y + 1).wall)
          walls.push_back({ scell.x, scell.y + 1 });
        if (scell.x < w2 - 2 && scell.y > 0 && scell.y < h2-2 && p_board->cell_at(scell.x + 1, scell.y).wall)
          walls.push_back({ scell.x + 1, scell.y });
        for (auto& wall : walls) {
          if (wall.x < 1 || wall.y < 1 || wall.x > w2-2 || wall.y > h2-2) {
            fmt::print(stderr, fmt_color, "gdzieś się wkradła zła ściana\n");
            return false;
          }
        }
      }
      auto it2 = it;
      ++it;
      if (it == walls.end())
        it = walls.begin();
      if (it == walls.end() || it == it2) {
        //koniec
        return finish();
      }
      walls.erase(it2);
    }
    skip:;
  }
  return true;
}



inline bool LabyrinthGenerator::finish() {
  not_end = false;
  p_board->cell_at(x,y).next_value = 0;
  return false;
}


int main(int argc, char** argv) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    return -1;
  }
  p_window = SDL_CreateWindow("420 colour it",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    420,
    420,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!p_window) {
    SDL_Quit();
    return -2;
  }
  SDL_GL_SetSwapInterval(0);
  context = SDL_GL_CreateContext(p_window);
  if (!context) {
    SDL_Quit();
    return -3;
  }
  if (glewInit() != GLEW_OK) {
    SDL_Quit();
    return -4;
  }

  load_shaders();

  Board board;
  board.set_size(420, 420);
  board.clear();
  board.gen_lab();

  std::vector<uint32_t> pixels(420 * 420);


  glClearColor(0,0,0,1);

  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(shp);

  glEnableVertexAttribArray(attr_vert);
  glEnableVertexAttribArray(attr_uv);
  glVertexAttribPointer(
    attr_vert,
    2,
    GL_FLOAT,
    GL_FALSE,
    0,
    verts
  );
  glVertexAttribPointer
  (
    attr_uv,
    2,
    GL_FLOAT,
    GL_FALSE,
    0,
    texuv
  );

  std::chrono::steady_clock::time_point one, two;
  one = std::chrono::steady_clock::now();

  bool not_quit = true;
  bool pause = false;
  while (not_quit) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) {
        not_quit = false;
      }
      if (ev.type == SDL_KEYDOWN) {
        if (ev.key.keysym.scancode == SDL_SCANCODE_R) {
          board.gen_lab();
        }
        if (ev.key.keysym.scancode == SDL_SCANCODE_SPACE) {
          pause = !pause;
        }
        if (ev.key.keysym.scancode == SDL_SCANCODE_S) {
          if (board.lgen.has_value())
            board.lgen->iters_left = 0;
        }
        if (ev.key.keysym.scancode == SDL_SCANCODE_E) {
          weird_rainbow = !weird_rainbow;
        }
      }
    }
    two = std::chrono::steady_clock::now();
    if (two - one > std::chrono::seconds(1) / ticrate) {
      one += std::chrono::seconds(1) / ticrate;
      if (!pause) {
        if (board.lgen.has_value()) {
          for (int i = 0; i < 120; ++i)
            board.advance_gen();
        } else {
          board.step();
        }
      }
      board.paste_texture(pixels.data());
    }
    draw(board.w, board.h, pixels.data());
  }

  SDL_GL_DeleteContext(context);
  SDL_Quit();

  return 0;
}