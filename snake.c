#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>

#define WIN_W 500
#define WIN_H 500
#define FPS 30
#define RES 50                  /* Number of rows and columns */
#define SNAKE_W (float) ((float) WIN_W / (float) RES)
#define SNAKE_H (float) ((float) WIN_H / (float) RES)

enum direction { RIGHT, DOWN, LEFT, UP };

struct snake {
  size_t x, y;
  enum direction dir;
  struct snake *tail;
};

struct apple {
  size_t cx, cy, r;
};

void draw_apple(struct apple *a) {
  DrawCircle(a->cx, a->cy, a->r, (Color) {220, 20, 60, 255});
}

bool snake_eats_apple(struct snake *s, struct apple *a) {
  Rectangle rec = {s->x * SNAKE_W, s->y * SNAKE_H, SNAKE_W, SNAKE_H};
  return CheckCollisionCircleRec((Vector2) {(float) a->cx, (float) a->cy},
                                 (float) a->r,
                                 rec);
}

bool collides_snake_head(int x, int y, struct snake *s) {
  if (!s) return false;
  if (x == s->x && y == s->y) return true;
  return collides_snake_head(x, y, s->tail);
}

void reposition_apple(struct apple *a) {
  a->r = GetRandomValue(5, 15);
  a->cx = GetRandomValue(a->r, WIN_W - a->r);
  a->cy = GetRandomValue(a->r, WIN_H - a->r);
}

/* Returns true if snake collided with itself or with the wall */
bool advance_snake(struct snake *snake, enum direction dir) {
  enum direction last_dir = snake->dir;

  switch (snake->dir) {
  case RIGHT:
    if (snake->x + 1 >= RES)
      snake->x = 0;
    else
      snake->x++;
    break;
  case DOWN:
    if (snake->y + 1 >= RES)
      snake->y = 0;
    else
      snake->y++;
    break;
  case LEFT:
    if (snake->x == 0) snake->x = RES;
    snake->x--;
    break;
  case UP:
    if (snake->y == 0) snake->y = RES;
    snake->y--;
    break;
  }

  snake->dir = dir;

  if (snake->tail) {
    advance_snake(snake->tail, last_dir);
    /* We check for collision after we move the snake's tail so that
       we don't falsely collide with the last tail. */
    if (collides_snake_head(snake->x, snake->y, snake->tail))
      return true;
  }

  return false;
}

/* Grow the snake from the head, not the tail */
struct snake *grow_snake(struct snake *snake) {
  if (!snake) return NULL;

  /* Don't forget to free the whole snake when the program ends! */
  struct snake *new = malloc(sizeof(struct snake));
  new->x = snake->x;
  new->y = snake->y;
  new->dir = snake->dir;

  new->tail = NULL;
  advance_snake(new, new->dir);
  new->tail = snake;

  return new;
}

void draw_snake(struct snake *snake, bool is_head) {
  if (!snake) return;
  DrawRectangle(snake->x * SNAKE_W, snake->y * SNAKE_H,
                SNAKE_W, SNAKE_H, is_head ? GREEN : WHITE);
  if (snake->tail) draw_snake(snake->tail, false);
}

int main(void) {
  InitWindow(WIN_W, WIN_H, "Snake");
  SetTargetFPS(FPS);

  size_t score = 0;
  bool collision = false;
  enum direction input_dir = RIGHT;

  struct apple apple;
  apple.r = 15;
  apple.cx = apple.cy = (WIN_W - apple.r) / 2;

  struct snake *snake = calloc(1, sizeof(struct snake));

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(collision ? RED : BLACK);

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
      if (input_dir != LEFT) input_dir = RIGHT;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
      if (input_dir != UP) input_dir = DOWN;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
      if (input_dir != RIGHT) input_dir = LEFT;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
      if (input_dir != DOWN) input_dir = UP;

    draw_apple(&apple);

    draw_snake(snake, true);

    if (!collision)
      collision = advance_snake(snake, input_dir);

    if (snake_eats_apple(snake, &apple)) {
      reposition_apple(&apple);
      snake = grow_snake(snake);
      score++;
      SetWindowTitle(TextFormat("Snake (%lu apple%s eaten)",
                                score, score > 1 ? "s" : ""));
    }

    EndDrawing();
  }

  /* Deallocating the snake */
  for (struct snake *cur = snake, *next; cur; cur = next) {
    next = cur->tail;
    free(cur);
  }

  CloseWindow();
  return EXIT_SUCCESS;
}
