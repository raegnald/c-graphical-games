#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <raylib.h>

#define FPS 60                  /* Frames per second */
#define WIN_W 860               /* Window width */
#define WIN_H 640               /* Window height */
#define BIRD_OFFSET 50          /* Bird's offset relative to the left
                                   border of the window */
#define BIRD_SIZE 10            /* Bird's size (square) */
#define GRAVITY 0.4             /* Measured in pixels per frame */
#define PROJECTILE_VEL 2        /* Velocity at which the bird goes up
                                   when space key is preddes */
#define BARRIER_SPEED 1         /* Measured in pixels per frame */
#define BARRIER_SEPARATION 300
#define MIN_BARRIER_HEIGHT 80
#define MAX_BARRIER_HEIGHT 180
#define MIN_BARRIER_Y_POS 50
#define BARRIERS_COUNT 3

/** A bird that flies upwards and downwards */
typedef struct {
  float ypos;                   /* Bird's position in the y axis */
  float vel;                    /* Bird's upwards velocity */
} Bird;

/** A barrier with a hole that the bird must pass */
typedef struct {
  float xpos;                   /* X position of the hole in the
                                   screen */
  float hole_start;             /* Y coordinate of the beginning of
                                   the hole */
  float hole_height;            /* The size of the hole */
  bool passed;                  /* Has the bird passed this barrier? */
} Barrier;


void generate_barrier_hole(Barrier *barrier) {
  size_t height = MIN_BARRIER_HEIGHT + rand() % (MAX_BARRIER_HEIGHT - MIN_BARRIER_HEIGHT);
  size_t start = MIN_BARRIER_Y_POS + rand() % (WIN_H - height - MIN_BARRIER_Y_POS);

  barrier->hole_height = (float) height;
  barrier->hole_start = (float) start;
}

void draw_barrier(Barrier *barrier, bool is_bird_dead) {
  /* Upper half */
  DrawRectangle(barrier->xpos, 0,
                BIRD_SIZE, barrier->hole_start,
                is_bird_dead ? BLACK : BROWN);

  /* Lower half */
  DrawRectangle(barrier->xpos,
                barrier->hole_start + barrier->hole_height,
                BIRD_SIZE,
                WIN_H -  barrier->hole_start - barrier->hole_height,
                is_bird_dead ? BLACK : BROWN);
}

void update_barrier(Barrier *barrier, double ticks) {
  float decr = BARRIER_SPEED + (ticks * 0.05);
  barrier->xpos -= decr > 5 ? 5 : decr;

  if (barrier->xpos + BIRD_SIZE < 0) {
    barrier->xpos = WIN_W;
    barrier->passed = false;
    generate_barrier_hole(barrier);
  }
}

void draw_bird(Bird *bird, bool is_dead) {
  DrawRectangle(BIRD_OFFSET, bird->ypos,
                BIRD_SIZE, BIRD_SIZE,
                is_dead ? WHITE : BLUE);
}

void update_bird(Bird *bird) {
  bird->vel += (float) GRAVITY;
  bird->vel *= 0.9;             /* Air resistance */
  bird->ypos += bird->vel;

  if (bird->ypos < 0) {
    bird->ypos = 0;
    bird->vel = 0;              /* Newton's third law */
  }

  if (bird->ypos + BIRD_SIZE > WIN_H) {
    bird->ypos = (float) (WIN_H - BIRD_SIZE);
    bird->vel = 0;              /* Newton's third law */
  }

}

bool check_bird_barrier_collision(Bird *bird, Barrier *barrier) {
  Rectangle bird_rect = {BIRD_OFFSET, bird->ypos, BIRD_SIZE, BIRD_SIZE};
  Rectangle upper_barrier_rect = {barrier->xpos, 0, BIRD_SIZE, barrier->hole_start};
  Rectangle lower_barrier_rect = {
    barrier->xpos,
    barrier->hole_start + barrier->hole_height,
    BIRD_SIZE,
    (float) WIN_H - barrier->hole_start - barrier->hole_height
  };

  return CheckCollisionRecs(bird_rect, upper_barrier_rect) || CheckCollisionRecs(bird_rect, lower_barrier_rect);
}

int main(void) {
  bool game_active = true;
  size_t score = 0;

  srand(time(NULL));
  InitWindow(WIN_W, WIN_H, "Flappy Bird C");
  SetTargetFPS(FPS);

  Bird bird = {
    .ypos = (float) WIN_H / 2,
    .vel  = 0
  };

  Barrier barriers[BARRIERS_COUNT] = {0};

  for (int i = 0; i < BARRIERS_COUNT; i++) {
    barriers[i].xpos = (BARRIER_SEPARATION + 1) * (i + 1);
    generate_barrier_hole(&barriers[i]);
  }

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(game_active ? WHITE : RED);

    if (IsKeyDown(KEY_SPACE)) {
      bird.vel -= PROJECTILE_VEL;
    }

    if (game_active) update_bird(&bird);
    draw_bird(&bird, !game_active);

    for (int i = 0; i < BARRIERS_COUNT; i++) {
      if (game_active) update_barrier(&barriers[i], GetTime());
      draw_barrier(&barriers[i], !game_active);

      /* Update score */
      if (barriers[i].xpos < BIRD_OFFSET && !barriers[i].passed) {
        score++;
        barriers[i].passed = true;
      }

      if (check_bird_barrier_collision(&bird, &barriers[i])) {
        game_active = false;
      }
    }

    DrawText(TextFormat("%lu", score), WIN_W / 2, BIRD_OFFSET, 50,
             game_active ? BLUE : WHITE);

    if (!game_active) {
      DrawText("Game over", BIRD_OFFSET, BIRD_OFFSET, 50, WHITE);
    }

    EndDrawing();
  }

  return EXIT_SUCCESS;
}
