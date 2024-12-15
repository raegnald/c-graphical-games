#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <raylib.h>
#include <raymath.h>

#define FPS 60                  /* Frames per second */
#define WIN_W 900               /* Window width */
#define WIN_H 600               /* Window height */

#define BLOCKS_Y_OFFSET 100     /* Space between start of blocks and
                                   top of window */
#define BLOCK_HEIGHT 20
#define BLOCKS_PER_ROW 10
#define AMOUNT_BLOCK_ROWS 8     /* How many rows of blocks are
                                   displayed */

#define RACKET_BOTTOM_OFFSET 50
#define RACKET_HEIGHT 10
#define RACKET_WIDTH 100
#define RACKET_COLOUR BLUE

#define BALL_RADIUS 10
#define BALL_COLOUR YELLOW

typedef struct {
  Rectangle rect;               /* Position and size of the block */
  Color color;                  /* The block's color */
  bool hit;                     /* Has the rectangle been hit by the
                                   ball? */
} Block;

typedef struct {
  Vector2 center;               /* Position of the ball */
  Vector2 vel;                  /* Ball's velocity */
} Ball;                         /* Ball that destroys blocks */

typedef struct {
  float start;                  /* Left of the racket (x-axis) */
} Racket;                       /* Player's racket */

Sound kick, hihat;

void draw_block(const Block *b) {
  DrawRectangleRec(b->rect, b->color);
}

Color random_light_color() {
  const char
    r = 100 + rand() % 156,
    g = 100 + rand() % 156,
    b = 100 + rand() % 156;

  return (Color) {r, g, b, 255};
}

void draw_racket(const Racket *r) {
  DrawRectangle(r->start, WIN_H - RACKET_HEIGHT - RACKET_BOTTOM_OFFSET,
                RACKET_WIDTH, RACKET_HEIGHT, RACKET_COLOUR);
}

void update_ball(Ball *b, Racket *racket, bool *game_active) {
  Rectangle racket_rect = {
    .x = racket->start,
    .y = WIN_H - RACKET_HEIGHT - RACKET_BOTTOM_OFFSET,
    .width = RACKET_WIDTH,
    .height = RACKET_HEIGHT
  };

  if (CheckCollisionCircleRec(b->center, BALL_RADIUS, racket_rect)) {
    PlaySound(hihat);
    b->vel.y *= -1;
  }

  if (b->center.x - BALL_RADIUS < 0 || b->center.x + BALL_RADIUS > WIN_W) {
    PlaySound(hihat);
    b->vel.x *= -1;
  }

  if (b->center.y - BALL_RADIUS < 0) {
    PlaySound(hihat);
    b->vel.y *= -1;
  }

  if (b->center.y + BALL_RADIUS > WIN_H)
    *game_active = false;

  b->center = Vector2Add(b->center, b->vel);
}

void draw_ball(const Ball *b) {
  DrawCircleV(b->center, BALL_RADIUS, BALL_COLOUR);
}

bool check_ball_block_collision(Ball *ball, Block *block) {
  bool collide = CheckCollisionCircleRec(ball->center, BALL_RADIUS, block->rect);

  if (collide) {
    ball->vel.y *= -1;
    if (fabs(ball->vel.y) < 10)
      ball->vel.y *= 1.1;
  }

  return collide;
}

int main(void) {
  bool game_active = true;
  bool game_won = false;
  size_t score = 0;

  srand(time(NULL));
  InitWindow(WIN_W, WIN_H, "Breakout C");
  InitAudioDevice();
  SetTargetFPS(FPS);

  kick = LoadSound("./sound/kick.mp3");
  hihat = LoadSound("./sound/hihat.mp3");

  SetSoundVolume(kick, 0.5);
  SetSoundVolume(hihat, 0.4);

  Music loop = LoadMusicStream("./sound/retro-wave.mp3");

  PlayMusicStream(loop);

  Block blocks[AMOUNT_BLOCK_ROWS][BLOCKS_PER_ROW];
  Racket racket = {0};
  Ball ball = {
    .center = (Vector2) {rand() % WIN_W, WIN_H - RACKET_BOTTOM_OFFSET - RACKET_HEIGHT - 20},
    .vel = (Vector2) {-(2 + rand() % 5), -(2 + rand() % 3)}
  };

  /* Initialising blocks */
  const int BLOCK_WIDTH = WIN_W / BLOCKS_PER_ROW;
  for (size_t row = 0; row < AMOUNT_BLOCK_ROWS; row++) {
    for (size_t col = 0; col < BLOCKS_PER_ROW; col++) {
      blocks[row][col] = (Block) {
        .rect = (Rectangle) {
          .x = col * BLOCK_WIDTH,
          .y = BLOCKS_Y_OFFSET + row * BLOCK_HEIGHT,
          .width = BLOCK_WIDTH,
          .height = BLOCK_HEIGHT
        },
        .color = random_light_color(),
        .hit = false
      };
    }
  }

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(game_won ? DARKGREEN : game_active ? BLACK : RED);

    UpdateMusicStream(loop);

    size_t blocks_alive, row;
    for (blocks_alive = 0, row = 0; row < AMOUNT_BLOCK_ROWS && !game_won; row++) {
      for (size_t col = 0; col < BLOCKS_PER_ROW; col++) {
        Block *block = &blocks[row][col];
        if (!block->hit) {
          if (check_ball_block_collision(&ball, block)) {
            PlaySound(kick);
            block->hit = true;
            score++;
          }
          draw_block(block);
          blocks_alive++;
        }
      }
    }
    if (blocks_alive == 0) game_won = true;

    /* Updating racket position based on mouse */
    racket.start = (float) GetMouseX() - (float) RACKET_WIDTH / 2;

    /* Checking bounds for racket position */
    if (racket.start < 0) racket.start = 0;
    if (racket.start + RACKET_WIDTH > WIN_W) racket.start = WIN_W - RACKET_WIDTH;

    draw_racket(&racket);

    if (game_active && !game_won) update_ball(&ball, &racket, &game_active);
    draw_ball(&ball);

    DrawText(TextFormat("%lu destroyed", score), 10, 10, 20,
             game_active ? YELLOW : WHITE);

    if (game_won)
      DrawText("You won!", 10, 30, 50, WHITE);
    else if (!game_active) {
      /* StopMusicStream(loop); */
      SetMasterVolume(0.33);
      DrawText("Game over", 10, 30, 50, WHITE);
    }

    EndDrawing();
  }

  return EXIT_SUCCESS;
}
