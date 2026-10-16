#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <raylib.h>

struct Enemy {
    Vector2 pos;
    float speed;
    bool alive;
};

struct Bullet {
    Vector2 pos;
    Vector2 vel;
    bool alive;
};

int main() {
    const int W = 800;
    const int H = 600;

    InitWindow(W, H, "Neon Shooter");
    SetTargetFPS(60);

    Vector2 player = {400, 300};
    int health = 5;
    int score = 0;
    float spawnTimer = 0;
    float shootTimer = 0;
    bool gameOver = false;

    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (!gameOver) {
            Vector2 move = {0, 0};

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
                move.y -= 1;
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
                move.y += 1;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
                move.x -= 1;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
                move.x += 1;

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                Vector2 target = GetMousePosition();
                float dx = target.x - player.x;
                float dy = target.y - player.y;
                float len = sqrtf(dx * dx + dy * dy);

                if (len > 0) {
                    move.x = dx / len;
                    move.y = dy / len;
                }
            }

            float len = sqrtf(
                move.x * move.x + move.y * move.y
            );

            if (len > 0) {
                player.x += move.x / len * 240 * dt;
                player.y += move.y / len * 240 * dt;
            }

            player.x = Clamp(player.x, 20, W - 20);
            player.y = Clamp(player.y, 20, H - 20);

            shootTimer -= dt;

            if (IsKeyDown(KEY_SPACE) &&
                shootTimer <= 0 &&
                !enemies.empty()) {

                int best = -1;
                float bestDist = 1e9f;

                for (int i = 0; i < (int)enemies.size(); i++) {
                    if (!enemies[i].alive) continue;

                    float dx = enemies[i].pos.x - player.x;
                    float dy = enemies[i].pos.y - player.y;
                    float d = sqrtf(dx * dx + dy * dy);

                    if (d < bestDist) {
                        bestDist = d;
                        best = i;
                    }
                }

                if (best >= 0) {
                    float dx = enemies[best].pos.x - player.x;
                    float dy = enemies[best].pos.y - player.y;
                    float d = sqrtf(dx * dx + dy * dy);

                    if (d > 0) {
                        bullets.push_back({
                            player,
                            {dx / d * 500, dy / d * 500},
                            true
                        });
                    }

                    shootTimer = 0.2f;
                }
            }

            spawnTimer += dt;

            if (spawnTimer > 1.2f) {
                spawnTimer = 0;

                int side = GetRandomValue(0, 3);
                Vector2 p;

                if (side == 0)
                    p = {(float)GetRandomValue(0, W), 0};
                else if (side == 1)
                    p = {(float)W, (float)GetRandomValue(0, H)};
                else if (side == 2)
                    p = {(float)GetRandomValue(0, W), (float)H};
                else
                    p = {0, (float)GetRandomValue(0, H)};

                enemies.push_back({
                    p,
                    (float)GetRandomValue(60, 100),
                    true
                });
            }

            for (auto &b : bullets) {
                if (!b.alive) continue;
                b.pos.x += b.vel.x * dt;
                b.pos.y += b.vel.y * dt;

                if (b.pos.x < 0 || b.pos.x > W ||
                    b.pos.y < 0 || b.pos.y > H)
                    b.alive = false;
            }

            for (auto &e : enemies) {
                if (!e.alive) continue;

                float dx = player.x - e.pos.x;
                float dy = player.y - e.pos.y;
                float d = sqrtf(dx * dx + dy * dy);

                if (d > 0) {
                    e.pos.x += dx / d * e.speed * dt;
                    e.pos.y += dy / d * e.speed * dt;
                }

                if (d < 30) {
                    e.alive = false;
                    health--;

                    if (health <= 0)
                        gameOver = true;
                }
            }

            for (auto &b : bullets) {
                if (!b.alive) continue;

                for (auto &e : enemies) {
                    if (!e.alive) continue;

                    float dx = b.pos.x - e.pos.x;
                    float dy = b.pos.y - e.pos.y;

                    if (sqrtf(dx * dx + dy * dy) < 20) {
                        b.alive = false;
                        e.alive = false;
                        score += 10;
                        break;
                    }
                }
            }

            for (int i = (int)bullets.size() - 1; i >= 0; i--)
                if (!bullets[i].alive)
                    bullets.erase(bullets.begin() + i);

            for (int i = (int)enemies.size() - 1; i >= 0; i--)
                if (!enemies[i].alive)
                    enemies.erase(enemies.begin() + i);
        }

        BeginDrawing();
        ClearBackground(Color{10, 15, 35, 255});

        for (int x = 0; x < W; x += 40)
            DrawLine(x, 0, x, H, DARKBLUE);

        for (int y = 0; y < H; y += 40)
            DrawLine(0, y, W, y, DARKBLUE);

        for (auto &b : bullets)
            DrawCircleV(b.pos, 5, YELLOW);

        for (auto &e : enemies) {
            DrawCircleV(e.pos, 18, RED);
            DrawCircleV(
                {e.pos.x - 5, e.pos.y - 4}, 3, WHITE
            );
            DrawCircleV(
                {e.pos.x + 5, e.pos.y - 4}, 3, WHITE
            );
        }

        DrawCircleV(player, 20, SKYBLUE);
        DrawCircleLines(
            (int)player.x, (int)player.y, 23, WHITE
        );

        DrawText(
            TextFormat("SCORE: %d", score),
            20, 20, 25, GOLD
        );

        DrawText(
            TextFormat("HEALTH: %d", health),
            20, 50, 22, GREEN
        );

        if (gameOver) {
            DrawRectangle(0, 0, W, H, Fade(BLACK, 0.8f));
            DrawText("GAME OVER", 240, 250, 45, RED);
            DrawText(
                TextFormat("FINAL SCORE: %d", score),
                260, 320, 25, WHITE
            );
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
