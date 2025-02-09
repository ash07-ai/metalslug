//https://github.com/ash07-ai/Metal-slug/tree/main
#include "raylib.h"
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <cstdlib>

using namespace std;



const int screenWidth = 1200;
const int screenHeight = 550;

Texture2D background, player_run1, player_run1_left, player_run2_left, player_run3_left, player_run2, player_run3, player_jump, player_fire;
Texture2D bulletTexture, tank_idle, tank_bullet, tank_dead, helicopter;
float scale = 0.04f;
Vector2 playerPos = { screenWidth / 100.0f - (player_run1.width * scale) / 2.0f, screenHeight - (player_run1.height * scale) - 130 };

int frame = 0;
float frameTime = 0.1f;
float elapsedTime = 0.0f;
bool isMoving = false, isJumping = false;
float jumpSpeed = 0.0f, gravity = 0.5f, groundLevel = playerPos.y;
bool isFiring = false;
float fireDuration = 0.2f;
float fireTimer = 0.0f;
int playerHealth = 100;
const int maxHealth = 100;

int bulletsShot = 0;          // Track how many bullets have been fired
float reloadTimer = 0.0f;     // Timer for reload cooldown
const float reloadTime = 2.0f; // Reload cooldown duration in seconds
bool isReloading = false;     // Flag to track reload state


int playerScore = 0;
struct Bullet {
    Vector2 position;
    bool active;
};

#define MAX_BULLETS 10
Bullet bullets[MAX_BULLETS] = { 0 };
typedef struct Enemy {
    bool active;
    Vector2 position;
    float fireTimer;
} Enemy;

#define MAX_ENEMIES 5
Enemy enemies[MAX_ENEMIES] = { 0 };
float bgX = 0;
float bgScale = 1.0f;
float bgWidth = 0;
float bgHeight = 0;
float levelEndX = 0;
struct TankBullet {
    Vector2 position;
    bool active;
};

#define MAX_TANK_BULLETS 10
TankBullet tankBullets[MAX_TANK_BULLETS] = { 0 };
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER
} GameState;

GameState gameState = GAME_STATE_PLAYING;

void UpdateReloadSystem(void)
{
    if (isReloading) {
        reloadTimer -= GetFrameTime();
        if (reloadTimer <= 0) {
            isReloading = false;
            bulletsShot = 0;  // Reset bullets shot
            printf("Reload complete!\n");
        }
    }
}

Vector2 helicopterPos = { 0 };
bool helicopterActive = true;
float helicopterSpeed = 2.0f;
struct Bomb {
    Vector2 position;
    bool active;
};

#define MAX_BOMBS 5
Bomb bombs[MAX_BOMBS] = { 0 };

void InitEnemyTextures() {
    tank_idle = LoadTexture("assets/tank_idle.png");
    tank_bullet = LoadTexture("assets/tank_bullet.png");
    tank_dead = LoadTexture("assets/tank_dead.png");
    helicopter = LoadTexture("assets/helicopter.png");

    if (tank_idle.width == 0 || tank_idle.height == 0) {
        printf("Failed to load tank_idle texture!\n");
    }
    else {
        printf("tank_idle texture loaded successfully! Dimensions: %d x %d\n", tank_idle.width, tank_idle.height);
    }

    if (tank_bullet.width == 0 || tank_bullet.height == 0) {
        printf("Failed to load tank_bullet texture!\n");
    }
    else {
        printf("tank_bullet texture loaded successfully!\n");
    }

    if (tank_dead.width == 0 || tank_dead.height == 0) {
        printf("Failed to load tank_dead texture!\n");
    }
    else {
        printf("tank_dead texture loaded successfully!\n");
    }
    if (helicopter.width == 0 || helicopter.height == 0) {
        printf("Failed to load helicopter texture!\n");
    }
    else {
        printf("helicopter texture loaded successfully! Dimensions: %d x %d\n", tank_idle.width, tank_idle.height);
    }

}



void InitTextures() {
    background = LoadTexture("assets/level2.png");
    player_run1 = LoadTexture("assets/player_run1.png");
    player_run1_left = LoadTexture("assets/player_run1_left.png");
    player_run2_left = LoadTexture("assets/player_run2_left.png");
    player_run3_left = LoadTexture("assets/player_run3_left.png");
    player_run2 = LoadTexture("assets/player_run2.png");
    player_run3 = LoadTexture("assets/player_run3.png");
    player_jump = LoadTexture("assets/player_jump.png");
    player_fire = LoadTexture("assets/player_fire.png");
    bulletTexture = LoadTexture("assets/bullet.png");
    InitEnemyTextures();

    if (bulletTexture.width == 0 || bulletTexture.height == 0) {
        printf("Failed to load bullet texture!\n");
    }
    else {
        printf("Bullet texture loaded successfully!\n");
    }
    float targetHeight = screenHeight;
    bgScale = targetHeight / background.height;
    bgWidth = background.width * bgScale;
    bgHeight = background.height * bgScale;
    levelEndX = bgWidth - screenWidth;

    printf("Background dimensions: %.2f x %.2f\n", bgWidth, bgHeight);
    printf("Level ends at: %.2f\n", levelEndX);
}

Sound PlayerbulletSound;
Sound tankMoveSound;
Sound explosionSound;
Sound playerJumpSound;
Sound playerHitSound;
Sound tankBulletSound;
Sound tankEngineSound;
Sound helicopterSound;
Sound bombDropSound;

void InitSounds() {
    // Initialize audio device
    InitAudioDevice();

    // Load sound effects
    PlayerbulletSound = LoadSound("assets/player_shoot.wav");       // Sound when player shoots
    tankMoveSound = LoadSound("assets/tank_moving.wav");        // Sound when tank moves
    explosionSound = LoadSound("assets/explosion.wav");       // Sound when tank is destroyed
    playerJumpSound = LoadSound("assets/player_jump.wav");    // Sound when player jumps
    tankBulletSound = LoadSound("assets/tank_bullet.wav");
    helicopterSound = LoadSound("assets/helicopter.wav");
    bombDropSound = LoadSound("assets/bomb_drop.wav");
    // Sound when player is hit

    // Optional: Set volume for each sound (0.0 to 1.0)
    SetSoundVolume(PlayerbulletSound, 0.5f);
    SetSoundVolume(tankMoveSound, 0.3f);
    SetSoundVolume(explosionSound, 0.7f);
    SetSoundVolume(playerJumpSound, 0.4f);
    SetSoundVolume(playerHitSound, 0.6f);
    SetSoundVolume(helicopterSound, 0.5f);
}

void CleanupSounds() {
    UnloadSound(PlayerbulletSound);
    UnloadSound(tankMoveSound);
    UnloadSound(explosionSound);
    UnloadSound(playerJumpSound);
    UnloadSound(playerHitSound);
    UnloadSound(tankBulletSound);
    UnloadSound(tankEngineSound);
    UnloadSound(helicopterSound);
    UnloadSound(bombDropSound);
    CloseAudioDevice();
}


bool isFacingLeft = false;
void UpdateMovement() {
    const float playerSpeed = 5.0f;
    const float scrollThreshold = screenWidth * 0.4f;

    bool enemyPresent = false;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemyPresent = true;
            break;
        }
    }


    if (IsKeyDown(KEY_RIGHT)) {
        isMoving = true;
        isFacingLeft = false;

        // If player hasn't reached the scroll threshold, move player
        if (playerPos.x < scrollThreshold) {
            playerPos.x += playerSpeed;
        }

        // Only scroll background if no enemy is present
        else if (-bgX < levelEndX && !enemyPresent) {
            bgX -= playerSpeed;
        }

        // If we're at the end of the level, allow player to move to the edge of the screen
        else if (playerPos.x + (player_run1.width * scale) < screenWidth) {
            playerPos.x += playerSpeed;
        }

        // Animation timing
        elapsedTime += GetFrameTime();
        if (elapsedTime >= frameTime) {
            elapsedTime = 0.0f;
            frame = (frame + 1) % 3;
        }
    }
    else if (IsKeyDown(KEY_LEFT)) {
        if (playerPos.x > 0) {
            playerPos.x -= playerSpeed;
            isMoving = true;
            isFacingLeft = true;

            elapsedTime += GetFrameTime();
            if (elapsedTime >= frameTime) {
                elapsedTime = 0.0f;
                frame = (frame + 1) % 3;
            }
        }
        else {
            // Change direction to right if at the left edge of the screen
            isFacingLeft = false;
        }
    }
    else {
        isMoving = false;
        frame = 0;
    }
    if (-bgX > levelEndX) {
        bgX = -levelEndX;
    }
    if (bgX > 0) {
        bgX = 0;
    }
}


void FireBullet(Vector2 playerPos)
{
    // Prevent firing if the player is facing left
    if (isFacingLeft) {
        return; // Exit the function without firing
    }
    if (!isReloading && bulletsShot < 2) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].position.x = playerPos.x + 20;
                bullets[i].position.y = playerPos.y + 20;
                bullets[i].active = true;
                isFiring = true;
                fireTimer = fireDuration;
                bulletsShot++;  // Increment bullets shot

                // Start reload timer if we've shot 2 bullets
                if (bulletsShot >= 2) {
                    isReloading = true;
                    reloadTimer = reloadTime;
                }

                printf("Bullet fired at position: (%f, %f), Bullets shot: %d\n",
                    bullets[i].position.x, bullets[i].position.y, bulletsShot);
                break;
            }
        }
    }
}

void UpdateBullets()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active) {
            bullets[i].position.x += 5;
            if (bullets[i].position.x > screenWidth) {
                bullets[i].active = false;
            }
            DrawTextureEx(bulletTexture, bullets[i].position, 2.9f, 0.06f, WHITE);
        }
    }
}

void SpawnTankBullet(Vector2 tankPosition) {
    for (int i = 0; i < MAX_TANK_BULLETS; i++) {
        if (!tankBullets[i].active) {
            tankBullets[i].position.x = tankPosition.x - 5;
            tankBullets[i].position.y = tankPosition.y;
            tankBullets[i].active = true;
            PlaySound(tankBulletSound);
            printf("Tank bullet fired at position: (%f, %f)\n", tankBullets[i].position.x, tankBullets[i].position.y);
            break;
        }
    }
}

void UpdateTankBullets()
{
    for (int i = 0; i < MAX_TANK_BULLETS; i++)
    {
        if (tankBullets[i].active)
        {
            tankBullets[i].position.x -= 5;

            if (tankBullets[i].position.x < 0) {
                tankBullets[i].active = false;
            }
            DrawTextureEx(tank_bullet, tankBullets[i].position, 2.0, 0.2f, WHITE);

            Rectangle playerRect = { playerPos.x, playerPos.y, player_run1.width * scale, player_run1.height * scale };
            Rectangle bulletRect = { tankBullets[i].position.x, tankBullets[i].position.y, tank_bullet.width * 0.1f, tank_bullet.height * 0.1f };

            if (CheckCollisionRecs(playerRect, bulletRect))
            {
                playerHealth -= 5;
                tankBullets[i].active = false;
                printf("Player hit by tank bullet!\n");
            }
        }
    }
}

Texture2D GetPlayerTexture() {
    if (isJumping) {
        return player_jump;
    }
    if (isFiring) {
        return player_fire;
    }
    if (isMoving) {
        if (frame == 0) {
            return isFacingLeft ? player_run1_left : player_run1;
        }
        else if (frame == 1) return isFacingLeft ? player_run2_left : player_run2;
        else return isFacingLeft ? player_run3_left : player_run3;
    }
    return isFacingLeft ? player_run1_left : player_run1;
}

void DrawBackground() {
    Rectangle source = { -bgX / bgScale,0,screenWidth / bgScale,background.height };

    Rectangle dest = { 0,0,screenWidth,bgHeight };
    Vector2 origin = { 0.0f, 0.0f };
    DrawTexturePro(background, source, dest, origin, 0, WHITE);
    DrawText(TextFormat("Background X: %.2f", bgX), 10, 40, 20, RED);
    DrawText(TextFormat("Level End X: %.2f", levelEndX), 10, 70, 20, RED);
}

float fireScale = 0.7f;

void DrawHealthBar() {
    float healthBarWidth = 200.0f;
    float healthPercentage = (float)playerHealth / maxHealth;
    DrawRectangle(10, 50, healthBarWidth, 20, RED);
    DrawRectangle(10, 50, healthBarWidth * healthPercentage, 20, GREEN);
    DrawText("HEALTH", 15, 51, 16, WHITE);
}

void DrawScoreBoard() {
    DrawText(TextFormat("Score: %d", playerScore), screenWidth - 150, 10, 20, BLACK);
}

void DrawPlayer() {
    Texture2D playerTexture = GetPlayerTexture();
    float appliedScale = (playerTexture.id == player_fire.id) ? fireScale : scale;
    DrawTextureEx(playerTexture, playerPos, 0.0f, appliedScale, WHITE);
}
bool canSpawnEnemy = true;
float spawnDelay = 2.0f;
float spawnTimer = 0.0f;

void SpawnEnemy() {
    bool enemyExists = false;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemyExists = true;
            break;
        }
    }
    if (!enemyExists) {
        spawnTimer += GetFrameTime();
        if (spawnTimer >= spawnDelay) {
            canSpawnEnemy = true;
        }
        if (canSpawnEnemy) {
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].active) {
                    enemies[i].position.x = screenWidth;
                    enemies[i].position.y = groundLevel - (tank_idle.height * 0.5f);
                    enemies[i].active = true;
                    canSpawnEnemy = false;
                    spawnTimer = 0.0f;

                    printf("Enemy spawned at position: (%.2f, %.2f)\n", enemies[i].position.x, enemies[i].position.y);
                    break;
                }
            }
        }
    }
}

void UpdateEnemies() {
    static float enemySpawnTimer = 0.0f;
    float tankFireInterval = 2.0f;
    float enemySpawnInterval = 5.0f;
    const float stopDistance = 400.0f;
    static int enemyHitCount[MAX_ENEMIES] = { 0 };
    bool activeEnemyExists = false;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            activeEnemyExists = true;
            break;
        }
    }
    if (!activeEnemyExists) {
        enemySpawnTimer += GetFrameTime();
    }
    if (!activeEnemyExists && enemySpawnTimer >= enemySpawnInterval) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                enemies[i].active = true;
                enemies[i].position.x = GetScreenWidth();
                enemies[i].position.y = groundLevel - (tank_idle.height * 0.3f) + 30;
                enemies[i].fireTimer = 0.0f;
                enemyHitCount[i] = 0;
                enemySpawnTimer = 0.0f;
                printf("New enemy spawned, hit count reset.\n");
                break;
            }
        }
    }
    const float scrollThreshold = screenWidth * 0.4f;
    bool isScrolling = (playerPos.x >= scrollThreshold && -bgX < levelEndX && IsKeyDown(KEY_RIGHT));
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            float distanceToPlayer = enemies[i].position.x - playerPos.x;
            if (distanceToPlayer > stopDistance) {
                enemies[i].position.x -= isScrolling ? 7.0f : 2.0f;
            }

            float desiredWidth = 100.0f;
            float enemyScale = desiredWidth / tank_idle.width;
            enemies[i].position.y = groundLevel - (tank_idle.height * enemyScale * 0.3f) + 30;

            DrawTextureEx(tank_idle, enemies[i].position, 0.0f, enemyScale, WHITE);
            enemies[i].fireTimer += GetFrameTime();
            if (distanceToPlayer <= stopDistance && enemies[i].fireTimer >= tankFireInterval) {
                SpawnTankBullet(enemies[i].position);
                enemies[i].fireTimer = 0.0f;
            }
            for (int j = 0; j < MAX_BULLETS; j++) {
                if (bullets[j].active) {
                    Rectangle bulletRect = { bullets[j].position.x, bullets[j].position.y, bulletTexture.width * 0.1f, bulletTexture.height * 0.1f };
                    Rectangle enemyRect = { enemies[i].position.x, enemies[i].position.y, tank_idle.width * enemyScale, tank_idle.height * enemyScale };

                    if (CheckCollisionRecs(bulletRect, enemyRect)) {
                        bullets[j].active = false;
                        enemyHitCount[i]++;
                        if (enemyHitCount[i] >= 3) {
                            enemies[i].active = false;
                            enemyHitCount[i] = 0;
                            playerScore += 1;
                            printf("Tank destroyed after 3 hits!\n");
                        }
                        else {
                            printf("Enemy hit! %d/3 hits received.\n", enemyHitCount[i]);
                        }
                    }
                }
            }
            Rectangle playerRect = {
                playerPos.x, playerPos.y,
                player_run1.width * scale, player_run1.height * scale
            };
            Rectangle enemyRect = {
                enemies[i].position.x, enemies[i].position.y,
                tank_idle.width * enemyScale, tank_idle.height * enemyScale
            };

            if (CheckCollisionRecs(playerRect, enemyRect)) {
                printf("Player hit by enemy!\n");
                enemies[i].active = false;
                playerHealth -= 20;

                if (playerHealth <= 0) {
                    printf("Game Over!\n");
                }
            }
            if (enemies[i].position.x + (tank_idle.width * enemyScale) < 0) {
                enemies[i].active = false;
                printf("Enemy went off screen. New enemy can now spawn after timer.\n");
            }
        }
    }
}

void UpdateJumping() {
    if (IsKeyPressed(KEY_J) && !isJumping) {
        isJumping = true;
        jumpSpeed = -10.0f;
    }
    if (isJumping)
    {
        playerPos.y += jumpSpeed;
        jumpSpeed += gravity;

        if (playerPos.y >= groundLevel) {
            playerPos.y = groundLevel;
            isJumping = false;
            jumpSpeed = 0.0f;
        }
    }
}

void DrawGameOver() {
    const char* gameOverText = "GAME OVER";
    const char* scoreText = TextFormat("Final Score: %d", playerScore);
    const char* restartText = "Press R to Restart";
    int gameOverTextWidth = MeasureText(gameOverText, 60);
    int scoreTextWidth = MeasureText(scoreText, 30);
    int restartTextWidth = MeasureText(restartText, 20);
    DrawRectangle(0, 0, screenWidth, screenHeight, RED);
    DrawText(gameOverText, (screenWidth - gameOverTextWidth) / 2, screenHeight / 3, 60, RED);
    DrawText(scoreText, (screenWidth - scoreTextWidth) / 2, screenHeight / 2, 30, WHITE);
    DrawText(restartText, (screenWidth - restartTextWidth) / 2, screenHeight * 2 / 3, 20, GRAY);
}

void UpdateHelicopter() {
    static float bombTimer = 0.0f;
    const float bombInterval = 2.0f;

    if (helicopterActive) {
        helicopterPos.x += helicopterSpeed;
        helicopterPos.y = 100;
        if (helicopterPos.x > screenWidth) {
            helicopterPos.x = -helicopter.width;
        }
        float helicopterScale = 0.9f;
        DrawTextureEx(helicopter, helicopterPos, 1.0f, helicopterScale, WHITE);
        bombTimer += GetFrameTime();
        if (bombTimer >= bombInterval) {
            for (int i = 0; i < MAX_BOMBS; i++) {
                if (!bombs[i].active) {
                    bombs[i].position.x = helicopterPos.x + helicopter.width / 2 * helicopterScale;
                    bombs[i].position.y = helicopterPos.y + helicopter.height * helicopterScale;
                    bombs[i].active = true;
                    break;
                }
            }
            bombTimer = 0.0f;
        }
    }
}

void UpdateHelicopterBombs() {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bombs[i].active) {
            bombs[i].position.y += 5.0f;
            DrawCircle(bombs[i].position.x, bombs[i].position.y, 10, BLACK);
            Rectangle playerRect = {
                playerPos.x, playerPos.y,
                player_run1.width * scale, player_run1.height * scale
            };

            Rectangle bombRect = {
                bombs[i].position.x - 5,
                bombs[i].position.y - 5,
                10, 10
            };
            if (CheckCollisionRecs(playerRect, bombRect)) {
                playerHealth -= 10;
                bombs[i].active = false;
            }
            if (bombs[i].position.y > screenHeight) {
                bombs[i].active = false;
            }
        }
    }
}


void InitPlayer() {
    playerPos.x = screenWidth / 100.0f - (player_run1.width * scale) / 2.0f;
    playerPos.y = screenHeight - (player_run1.height * scale) - 50;
    groundLevel = playerPos.y;
}

void ResetGame() {
    playerHealth = maxHealth;
    playerScore = 0;
    playerPos.x = screenWidth / 100.0f - (player_run1.width * scale) / 2.0f;
    playerPos.y = screenHeight - (player_run1.height * scale) - 50;
    groundLevel = playerPos.y;
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
    for (int i = 0; i < MAX_TANK_BULLETS; i++) {
        tankBullets[i].active = false;
    }

    gameState = GAME_STATE_PLAYING;
}


void DrawReloadStatus(void)
{
    if (isReloading) {
        DrawText(TextFormat("RELOADING... %.1f", reloadTimer), 10, 100, 20, RED);
    }
    else {
        DrawText(TextFormat("AMMO: %d/2", 2 - bulletsShot), 10, 100, 20, BLACK);
    }
}


void InitEnemyTextures1() {
    tank_idle = LoadTexture("assets/tank_idle.png");
    tank_bullet = LoadTexture("assets/tank_bullet.png");
    tank_dead = LoadTexture("assets/tank_dead.png");
}

void InitTextures1() {
    background = LoadTexture("assets/bg.png");
    player_run1 = LoadTexture("assets/player_run1.png");
    player_run1_left = LoadTexture("assets/player_run1_left.png");
    player_run2_left = LoadTexture("assets/player_run2_left.png");
    player_run3_left = LoadTexture("assets/player_run3_left.png");
    player_run2 = LoadTexture("assets/player_run2.png");
    player_run3 = LoadTexture("assets/player_run3.png");
    player_jump = LoadTexture("assets/player_jump.png");
    player_fire = LoadTexture("assets/player_fire.png");
    bulletTexture = LoadTexture("assets/bullet.png");
    InitEnemyTextures1();
    float targetHeight = screenHeight;
    bgScale = targetHeight / background.height;
    bgWidth = background.width * bgScale;
    bgHeight = background.height * bgScale;
    levelEndX = bgWidth - screenWidth;

}



void main1() {
    InitWindow(screenWidth, screenHeight, "Raylib - Background Adjusted");
    int count = 0;
    InitTextures1();
    InitPlayer();
    SetTargetFPS(60);
    float enemySpawnTimer = 0.0f;
    float enemySpawnInterval = 2.0f;

    while (!WindowShouldClose()) {
        if (gameState == GAME_STATE_PLAYING) {
            UpdateMovement();
            UpdateJumping();
            UpdateReloadSystem();

            if (IsKeyPressed(KEY_SPACE) && count <= 5)
            {
                count++;
                FireBullet(playerPos);
            }
            if (count > 5)
            {
                count = 0;
            }

            if (isFiring) {
                fireTimer -= GetFrameTime();
                if (fireTimer <= 0.9f) {
                    isFiring = false;
                }
            }
            enemySpawnTimer += GetFrameTime();
            if (enemySpawnTimer >= enemySpawnInterval) {
                SpawnEnemy();
                enemySpawnTimer = 0.0f;
            }

            UpdateBullets();
            UpdateEnemies();
            UpdateTankBullets();
            if (playerHealth <= 0) {
                gameState = GAME_STATE_GAME_OVER;
            }
        }
        else if (gameState == GAME_STATE_GAME_OVER) {
            if (IsKeyPressed(KEY_R)) {
                ResetGame();
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (gameState == GAME_STATE_PLAYING) {
            DrawBackground();
            UpdateEnemies();
            UpdateBullets();
            UpdateTankBullets();
            DrawPlayer();
            DrawHealthBar();
            DrawScoreBoard();

            DrawReloadStatus();

            DrawText("Use Left/Right arrows to move! Press 'J' to Jump and Space to Shoot.", 10, 10, 20, DARKGRAY);
        }
        else if (gameState == GAME_STATE_GAME_OVER) {
            DrawGameOver();
        }

        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(player_run1);
    UnloadTexture(player_run2);
    UnloadTexture(player_run3);
    UnloadTexture(player_jump);
    UnloadTexture(bulletTexture);
    UnloadTexture(player_fire);

    CloseWindow();
}



//Vector2 helicopterPos = { 0 };
//bool helicopterActive = true;
//float helicopterSpeed = 2.0f;
//struct Bomb {
//    Vector2 position;
//    bool active;
//};
//
//#define MAX_BOMBS 5
//Bomb bombs[MAX_BOMBS] = { 0 };
//Texture2D bulletTexture2, tank_idle2, tank_bullet2, tank_dead2, helicopter;
//Texture2D background2;

void InitEnemyTextures2() {
    tank_idle = LoadTexture("assets/tank_idle.png");
    tank_bullet = LoadTexture("assets/tank_bullet.png");
    tank_dead = LoadTexture("assets/tank_dead.png");
    helicopter = LoadTexture("assets/helicopter.png");

    if (tank_idle.width == 0 || tank_idle.height == 0) {
        printf("Failed to load tank_idle texture!\n");
    }
    else {
        printf("tank_idle texture loaded successfully! Dimensions: %d x %d\n", tank_idle.width, tank_idle.height);
    }

    if (tank_bullet.width == 0 || tank_bullet.height == 0) {
        printf("Failed to load tank_bullet texture!\n");
    }
    else {
        printf("tank_bullet texture loaded successfully!\n");
    }

    if (tank_dead.width == 0 || tank_dead.height == 0) {
        printf("Failed to load tank_dead texture!\n");
    }
    else {
        printf("tank_dead texture loaded successfully!\n");
    }
    if (helicopter.width == 0 || helicopter.height == 0) {
        printf("Failed to load helicopter texture!\n");
    }
    else {
        printf("helicopter texture loaded successfully! Dimensions: %d x %d\n", tank_idle.width, tank_idle.height);
    }

}



void InitTextures2() {
    background = LoadTexture("assets/level2.png");
    player_run1 = LoadTexture("assets/player_run1.png");
    player_run1_left = LoadTexture("assets/player_run1_left.png");
    player_run2_left = LoadTexture("assets/player_run2_left.png");
    player_run3_left = LoadTexture("assets/player_run3_left.png");
    player_run2 = LoadTexture("assets/player_run2.png");
    player_run3 = LoadTexture("assets/player_run3.png");
    player_jump = LoadTexture("assets/player_jump.png");
    player_fire = LoadTexture("assets/player_fire.png");
    bulletTexture = LoadTexture("assets/bullet.png");
    InitEnemyTextures2();

    if (bulletTexture.width == 0 || bulletTexture.height == 0) {
        printf("Failed to load bullet texture!\n");
    }
    else {
        printf("Bullet texture loaded successfully!\n");
    }
    float targetHeight = screenHeight;
    bgScale = targetHeight / background.height;
    bgWidth = background.width * bgScale;
    bgHeight = background.height * bgScale;
    levelEndX = bgWidth - screenWidth;

    printf("Background dimensions: %.2f x %.2f\n", bgWidth, bgHeight);
    printf("Level ends at: %.2f\n", levelEndX);
}



int main2() {
    InitWindow(screenWidth, screenHeight, "Raylib - Background Adjusted");



    InitTextures2();
    InitPlayer();
    SetTargetFPS(60);


    float enemySpawnTimer = 0.0f;
    float enemySpawnInterval = 2.0f;

    while (!WindowShouldClose()) {
        int count = 0;
        if (gameState == GAME_STATE_PLAYING)
        {

            UpdateMovement();
            UpdateJumping();
            UpdateReloadSystem();

            if (IsKeyPressed(KEY_SPACE) && count <= 5)
            {
                count++;
                FireBullet(playerPos);
            }

            if (isFiring) {
                fireTimer -= GetFrameTime();
                if (fireTimer <= 0.9f) {
                    isFiring = false;
                }
            }
            enemySpawnTimer += GetFrameTime();
            if (enemySpawnTimer >= enemySpawnInterval) {
                SpawnEnemy();
                enemySpawnTimer = 0.0f;
            }

            UpdateBullets();
            UpdateEnemies();
            UpdateTankBullets();
            if (playerHealth <= 0) {
                gameState = GAME_STATE_GAME_OVER;
            }
        }
        else if (gameState == GAME_STATE_GAME_OVER) {
            if (IsKeyPressed(KEY_R)) {
                ResetGame();
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (gameState == GAME_STATE_PLAYING)
        {
            DrawBackground();
            UpdateEnemies();

            UpdateHelicopter();
            UpdateHelicopterBombs();

            UpdateBullets();
            UpdateTankBullets();
            DrawPlayer();
            DrawHealthBar();
            DrawScoreBoard();

            DrawText("Use Left/Right arrows to move! Press 'J' to Jump and Space to Shoot.", 10, 10, 20, DARKGRAY);

            if (bulletTexture.width == 0 || bulletTexture.height == 0) {
                DrawText("Error loading bullet texture!", 200, 200, 20, RED);
            }
        }
        else if (gameState == GAME_STATE_GAME_OVER) {
            DrawGameOver();
        }

        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(player_run1);
    UnloadTexture(player_run2);
    UnloadTexture(player_run3);
    UnloadTexture(player_jump);
    UnloadTexture(bulletTexture);
    UnloadTexture(player_fire);

    CloseWindow();
    return 0;
}



Texture2D background_3, player_run1_3, player_run1_left_3, player_run2_left_3, player_fire_up_3, player_run3_left_3, player_run2_3, player_run3_3, player_jump_3, player_fire_3;
Texture2D bulletTexture_3, boss_3, boss_bullet_3, ufo_3, ufo_bullet_3;

float scale_3 = 0.04f;
Vector2 playerPos_3 = { screenWidth / 100.0f, screenHeight - 200 };

int frame_3 = 0;
float frameTime_3 = 0.1f;
float elapsedTime_3 = 0.0f;
bool isMoving_3 = false, isJumping_3 = false;
float jumpSpeed_3 = 0.0f, gravity_3 = 0.5f, groundLevel_3 = playerPos_3.y;

bool isFiring_3 = false;
float fireDuration_3 = 0.2f;
float fireTimer_3 = 0.0f;

int playerHealth_3 = 100;
const int maxHealth_3 = 100;
int playerScore_3 = 0;

struct Bullet_3 {
    Vector2 position_3;
    bool active_3;
};

#define MAX_BULLETS_3 10
Bullet_3 bullets_3[MAX_BULLETS_3] = { 0 };

struct BossBullet_3 {
    Vector2 position_3;
    bool active_3;
};

#define MAX_BOSS_BULLETS_3 3
BossBullet_3 bossBullets_3[MAX_BOSS_BULLETS_3] = { 0 };

typedef enum {
    GAME_STATE_PLAYING_3,
    GAME_STATE_BOSS_FIGHT_3,
    GAME_STATE_GAME_OVER_3
} GameState_3;

GameState_3 gameState_3 = GAME_STATE_PLAYING_3;

Vector2 bossPos_3 = {
    screenWidth - (boss_3.width * 0.3f) - 400,  
    screenHeight * 0.4f - 100 
};

struct UFO_3 {
    Vector2 position_3;
    bool active_3;
    int health_3;
    float bulletTimer_3;
};

struct UFOBullet_3 {
    Vector2 position_3;
    bool active_3;
};

UFO_3 ufoEnemy_3 = { 0 };
#define MAX_UFO_BULLETS_3 5
UFOBullet_3 ufoBullets_3[MAX_UFO_BULLETS_3] = { 0 };
bool isAimingUp_3 = false;
float ufoMoveTimer_3 = 0.0f;
float ufoMoveInterval_3 = 1.0f;
float ufoSpeed_3 = 3.0f;
int ufoDirection_3 = 1;

typedef struct {
    float targetX_3;
    float targetY_3;
    float currentSpeed_3;
    float angle_3;
    float moveTimer_3;
} UFOMovement_3;

#define MAX_UFOS_3 5
UFO_3 ufos_3[MAX_UFOS_3] = { 0 };
int ufoDefeatedCount_3 = 0;

UFOMovement_3 ufoMovements_3[MAX_UFOS_3] = { 0 };

const float MIN_SPEED_3 = 2.0f;
const float MAX_SPEED_3 = 4.0f;
const float SMOOTH_FACTOR_3 = 0.05f;
const float DIRECTION_CHANGE_TIME_3 = 2.0f;

Vector2 GetNewUFOTarget_3(int ufoIndex_3) {
    float margin_3 = ufo_3.width * 0.3f;
    return
        Vector2{
        (float)GetRandomValue((int)margin_3, (int)screenWidth - margin_3),
          (float)GetRandomValue((int)screenHeight * 0.1f,(int)screenHeight * 0.4f)
    };
}

float bossScale_3 = 0.2f;
int bossHealth_3 = 100;
float bossBulletTimer_3 = 0.0f;
int bossBulletCount_3 = 0;
const int MAX_BOSS_BULLET_VOLLEYS_3 = 3;

void InitTextures_3() {
    background_3 = LoadTexture("assets/background_1.png");
    player_run1_3 = LoadTexture("assets/player_run1.png");
    player_run1_left_3 = LoadTexture("assets/player_run1_left.png");
    player_run2_left_3 = LoadTexture("assets/player_run2_left.png");
    player_run3_left_3 = LoadTexture("assets/player_run3_left.png");
    player_run2_3 = LoadTexture("assets/player_run2.png");
    player_run3_3 = LoadTexture("assets/player_run3.png");
    player_jump_3 = LoadTexture("assets/player_jump.png");
    player_fire_3 = LoadTexture("assets/player_fire.png");
    player_fire_up_3 = LoadTexture("assets/player_fire_up.png");
    bulletTexture_3 = LoadTexture("assets/bullet.png");
    boss_3 = LoadTexture("assets/boss.png");
    boss_bullet_3 = LoadTexture("assets/boss_bullet.png");
    ufo_3 = LoadTexture("assets/ufo.png");
    ufo_bullet_3 = LoadTexture("assets/ufo_bullet.png");
}

bool isFacingLeft_3 = false;

int bulletsShot_3 = 0;
float reloadTimer_3 = 0.0f;
const float reloadTime_3 = 2.0f;
bool isReloading_3 = false;

void UpdateReloadSystem_3(void) {
    if (isReloading_3) {
        reloadTimer_3 -= GetFrameTime();
        if (reloadTimer_3 <= 0) {
            isReloading_3 = false;
            bulletsShot_3 = 0;
        }
    }
}

void UpdateBullets_3() {
    for (int i = 0; i < MAX_BULLETS_3; i++) {
        if (bullets_3[i].active_3) {
            if (isAimingUp_3) {
                bullets_3[i].position_3.y -= 10;
            }
            else {
                bullets_3[i].position_3.x += 10;
            }

            if (bullets_3[i].position_3.x > screenWidth || bullets_3[i].position_3.y < 0) {
                bullets_3[i].active_3 = false;
            }

            
            for (int j = 0; j < MAX_UFOS_3; j++) {
                if (ufos_3[j].active_3) {
                    Rectangle ufoRect_3 = {
                        ufos_3[j].position_3.x,
                        ufos_3[j].position_3.y,
                        ufo_3.width * 0.3f,
                        ufo_3.height * 0.3f
                    };
                    Rectangle bulletRect_3 = {
                        bullets_3[i].position_3.x,
                        bullets_3[i].position_3.y,
                        bulletTexture_3.width * 0.1f,
                        bulletTexture_3.height * 0.1f
                    };

                    if (CheckCollisionRecs(ufoRect_3, bulletRect_3)) {
                        ufos_3[j].health_3 -= 10;
                        bullets_3[i].active_3 = false;
                        if (ufos_3[j].health_3 <= 0) {
                            ufos_3[j].active_3 = false;
                            playerScore_3 += 1000;
                            ufoDefeatedCount_3++;
                        }
                        break;
                    }
                }
            }


            if (ufoDefeatedCount_3 >= 3 && gameState_3 == GAME_STATE_BOSS_FIGHT_3) {
                Rectangle bossRect_3 = {
                    bossPos_3.x,
                    bossPos_3.y,
                    boss_3.width * 0.5f,
                    boss_3.height * 0.5f
                };
                Rectangle bulletRect_3 = {
                    bullets_3[i].position_3.x,
                    bullets_3[i].position_3.y,
                    bulletTexture_3.width * 0.1f,
                    bulletTexture_3.height * 0.1f
                };

                if (CheckCollisionRecs(bossRect_3, bulletRect_3)) {
                    bossHealth_3 -= 10;
                    bullets_3[i].active_3 = false;
                }
            }

            DrawTextureEx(bulletTexture_3, bullets_3[i].position_3, 2.9f, 0.05f, WHITE);
        }
    }
}




void FireBullet_3(Vector2 playerPos_3) {

   
    if (isFacingLeft_3) {
        return; 
    }
    if (!isReloading_3 && bulletsShot_3 < 5) {
        for (int i = 0; i < MAX_BULLETS_3; i++) {
            if (!bullets_3[i].active_3) {
                bullets_3[i].position_3.x = playerPos_3.x + 20;
                bullets_3[i].position_3.y = playerPos_3.y + 20;
                bullets_3[i].active_3 = true;
                isFiring_3 = true;
                fireTimer_3 = fireDuration_3;
                bulletsShot_3++;  


                if (bulletsShot_3 >= 4) {
                    isReloading_3 = true;
                    reloadTimer_3 = reloadTime_3;
                }

                printf("Bullet fired at position: (%f, %f), Bullets shot: %d\n",
                    bullets_3[i].position_3.x, bullets_3[i].position_3.y, bulletsShot_3);
                break;
            }
        }
    }
}


void InitAllUFOs_3() {
    float spacing_3 = screenWidth / (MAX_UFOS_3 + 1);
    for (int i = 0; i < MAX_UFOS_3; i++) {

        ufos_3[i].position_3 = Vector2{ spacing_3 * (i + 1), -20 };
        ufos_3[i].active_3 = true;
        ufos_3[i].health_3 = 50;
        ufos_3[i].bulletTimer_3 = 0;


        ufoMovements_3[i].targetX_3 = spacing_3 * (i + 1);
        ufoMovements_3[i].targetY_3 = screenHeight * 0.2f;
        ufoMovements_3[i].currentSpeed_3 = MIN_SPEED_3;
        ufoMovements_3[i].angle_3 = 0;
        ufoMovements_3[i].moveTimer_3 = GetRandomValue(0, 100) / 100.0f * DIRECTION_CHANGE_TIME_3;
    }
    ufoDefeatedCount_3 = 0;
}

void UpdateUFOs_3() {
    for (int i = 0; i < MAX_UFOS_3; i++) {
        if (!ufos_3[i].active_3) continue;

        ufoMovements_3[i].moveTimer_3 += GetFrameTime();

        if (ufoMovements_3[i].moveTimer_3 >= DIRECTION_CHANGE_TIME_3) {
            Vector2 newTarget_3 = GetNewUFOTarget_3(i);
            ufoMovements_3[i].targetX_3 = newTarget_3.x;
            ufoMovements_3[i].targetY_3 = newTarget_3.y;
            ufoMovements_3[i].currentSpeed_3 = MIN_SPEED_3 + (MAX_SPEED_3 - MIN_SPEED_3) * (GetRandomValue(0, 100) / 100.0f);
            ufoMovements_3[i].moveTimer_3 = 0;
        }

        float dx_3 = ufoMovements_3[i].targetX_3 - ufos_3[i].position_3.x;
        float dy_3 = ufoMovements_3[i].targetY_3 - ufos_3[i].position_3.y;


        float distance_3 = sqrt(dx_3 * dx_3 + dy_3 * dy_3);

        if (distance_3 > 1.0f) {  

            float moveX_3 = (dx_3 / distance_3) * ufoMovements_3[i].currentSpeed_3;
            float moveY_3 = (dy_3 / distance_3) * ufoMovements_3[i].currentSpeed_3;


            ufos_3[i].position_3.x += moveX_3;
            ufos_3[i].position_3.y += moveY_3;
        }


        float margin_3 = ufo_3.width * 0.3f;
        ufos_3[i].position_3.x = fmax(margin_3, fmin(ufos_3[i].position_3.x, screenWidth - margin_3));
        ufos_3[i].position_3.y = fmax(screenHeight * 0.1f, fmin(ufos_3[i].position_3.y, screenHeight * 0.4f));

        ufos_3[i].bulletTimer_3 += GetFrameTime();
        if (ufos_3[i].bulletTimer_3 >= 1.5f) {
            for (int j = 0; j < MAX_UFO_BULLETS_3; j++) {
                if (!ufoBullets_3[j].active_3) {
                    ufoBullets_3[j].position_3 = Vector2{
                        ufos_3[i].position_3.x + (ufo_3.width * 0.15f),
                        ufos_3[i].position_3.y + (ufo_3.height * 0.3f)
                    };
                    ufoBullets_3[j].active_3 = true;
                    break;
                }
            }
            ufos_3[i].bulletTimer_3 = 0;
        }


        ufoMovements_3[i].angle_3 += 0.05f;
        float wobbleOffset_3 = sin(ufoMovements_3[i].angle_3) * 2.0f;


        Vector2 drawPos_3 = ufos_3[i].position_3;
        drawPos_3.y += wobbleOffset_3;
        DrawTextureEx(ufo_3, drawPos_3, 0, 0.4f, WHITE);
    }
}






void UpdateMovement_3() {

    const float playerSpeed_3 = 5.0f;

    isAimingUp_3 = IsKeyDown(KEY_UP);

    if (IsKeyPressed(KEY_SPACE)) {
        FireBullet_3(playerPos_3);
        isFiring_3 = true;
        fireTimer_3 = fireDuration_3;
    }

    if (IsKeyDown(KEY_RIGHT)) {
        isMoving_3 = true;
        isFacingLeft_3 = false;
        if (playerPos_3.x + (player_run1_3.width * scale_3) < screenWidth) {
            playerPos_3.x += playerSpeed_3;
        }
        elapsedTime_3 += GetFrameTime();
        if (elapsedTime_3 >= frameTime_3) {
            elapsedTime_3 = 0.0f;
            frame_3 = (frame_3 + 1) % 3;
        }
    }
    else if (IsKeyDown(KEY_LEFT)) {
        if (playerPos_3.x > 0) {
            playerPos_3.x -= playerSpeed_3;
            isMoving_3 = true;
            isFacingLeft_3 = true;
            elapsedTime_3 += GetFrameTime();
            if (elapsedTime_3 >= frameTime_3) {
                elapsedTime_3 = 0.0f;
                frame_3 = (frame_3 + 1) % 3;
            }
        }
    }
    else {
        isMoving_3 = false;
        frame_3 = 0;
    }


    if (isFiring_3) {
        fireTimer_3 -= GetFrameTime();
        if (fireTimer_3 <= 0) {
            isFiring_3 = false;
        }
    }
}


void UpdateUFOBullets() {
    for (int i = 0; i < MAX_UFO_BULLETS_3; i++) {
        if (ufoBullets_3[i].active_3) {
            ufoBullets_3[i].position_3.y += 5;


            Rectangle playerRect_3 = {
                playerPos_3.x,
                playerPos_3.y,
                player_run1_3.width * scale_3,
                player_run1_3.height * scale_3
            };
            Rectangle bulletRect_3 = {
                ufoBullets_3[i].position_3.x,
                ufoBullets_3[i].position_3.y,
                ufo_bullet_3.width * 0.2f,
                ufo_bullet_3.height * 0.2f
            };

            if (CheckCollisionRecs(playerRect_3, bulletRect_3)) {
                playerHealth_3 -= 4;
                ufoBullets_3[i].active_3 = false;
            }

            if (ufoBullets_3[i].active_3) {
                DrawTextureEx(ufo_bullet_3, ufoBullets_3[i].position_3, 0, 0.7f, WHITE);
            }


            if (ufoBullets_3[i].position_3.y > screenHeight) {
                ufoBullets_3[i].active_3 = false;
            }
        }
    }
}


void UpdateBossBullets_3() {
    for (int i = 0; i < MAX_BOSS_BULLETS_3; i++) {
        if (bossBullets_3[i].active_3) {
            bossBullets_3[i].position_3.x -= 5;

            if (bossBullets_3[i].position_3.x < 0) {
                bossBullets_3[i].active_3 = false;
            }

            Rectangle playerRect_3 = {
                playerPos_3.x,
                playerPos_3.y,
                player_run1_3.width * scale_3,
                player_run1_3.height * scale_3
            };
            Rectangle bulletRect_3 = {
                bossBullets_3[i].position_3.x,
                bossBullets_3[i].position_3.y,
                boss_bullet_3.width * 0.9f, 
                boss_bullet_3.height * 0.5f
            };

            if (CheckCollisionRecs(playerRect_3, bulletRect_3)) {
                playerHealth_3 -= 10;
                bossBullets_3[i].active_3 = false;
            }


            DrawTextureEx(boss_bullet_3, bossBullets_3[i].position_3, 0, 0.2f, WHITE);
        }
    }
}


void FireBossBullets_3() {
    if (bossBulletCount_3 < MAX_BOSS_BULLET_VOLLEYS_3) {
        for (int i = 0; i < MAX_BOSS_BULLETS_3; i++) {
            if (!bossBullets_3[i].active_3) {
       
                bossBullets_3[i].position_3.x = bossPos_3.x - (boss_bullet_3.width * 0.5f) + 200;
                bossBullets_3[i].position_3.y = bossPos_3.y + (boss_3.height * bossScale_3 / 2) + 10;
                bossBullets_3[i].active_3 = true;
            }
        }
        bossBulletCount_3++;
    }
}

Texture2D GetPlayerTexture_3() {
    if (isAimingUp_3) return player_fire_up_3;
    if (isJumping_3) return player_jump_3;
    if (isFiring_3) return player_run1_3;
    if (isMoving_3) {
        if (frame_3 == 0) return isFacingLeft_3 ? player_run1_left_3 : player_run1_3;
        else if (frame_3 == 1) return isFacingLeft_3 ? player_run2_left_3 : player_run2_3;
        else return isFacingLeft_3 ? player_run3_left_3 : player_run3_3;
    }
    return isFacingLeft_3 ? player_run1_left_3 : player_run1_3;
}

void DrawHealthBar_3() {
    float healthBarWidth_3 = 200.0f;
    float healthPercentage_3 = (float)playerHealth_3 / maxHealth_3;
    DrawRectangle(10, 50, healthBarWidth_3, 20, RED);
    DrawRectangle(10, 50, healthBarWidth_3 * healthPercentage_3, 20, GREEN);
    DrawText("PLAYER", 15, 51, 16, WHITE);

    float bossHealthBarWidth_3 = 200.0f;
    float bossHealthPercentage_3 = (float)bossHealth_3 / 100;
    DrawRectangle(screenWidth - 210, 50, bossHealthBarWidth_3, 20, RED);
    DrawRectangle(screenWidth - 210, 50, bossHealthBarWidth_3 * bossHealthPercentage_3, 20, GREEN);
    DrawText("BOSS", screenWidth - 205, 51, 16, WHITE);
}

void DrawPlayer_3() {
    Texture2D playerTexture_3 = GetPlayerTexture_3();
    float appliedScale_3 = (playerTexture_3.id == player_fire_3.id) ? 0.7f : scale_3;
    DrawTextureEx(playerTexture_3, playerPos_3, 0.0f, appliedScale_3, WHITE);
}

void UpdateJumping_3() {
    if (IsKeyPressed(KEY_J) && !isJumping_3) {
        isJumping_3 = true;
        jumpSpeed_3 = -10.0f;
    }

    if (isJumping_3) {
        playerPos_3.y += jumpSpeed_3;
        jumpSpeed_3 += gravity_3;

        if (playerPos_3.y >= groundLevel_3) {
            playerPos_3.y = groundLevel_3;
            isJumping_3 = false;
            jumpSpeed_3 = 0.0f;
        }
    }
}

void DrawGameOver_3() {
    const char* gameOverText_3 = bossHealth_3 <= 0 ? "YOU WIN!" : "GAME OVER";
    const char* scoreText_3 = TextFormat("Final Score: %d", playerScore_3);
    const char* restartText_3 = "Press R to Restart";
    DrawRectangle(0, 0, screenWidth, screenHeight, bossHealth_3 <= 0 ? GREEN : RED);
    DrawText(gameOverText_3, (screenWidth - MeasureText(gameOverText_3, 60)) / 2, screenHeight / 3, 60, WHITE);
    DrawText(scoreText_3, (screenWidth - MeasureText(scoreText_3, 30)) / 2, screenHeight / 2, 30, WHITE);
    DrawText(restartText_3, (screenWidth - MeasureText(restartText_3, 20)) / 2, screenHeight * 2 / 3, 20, GRAY);
}

void ResetGame_3() {
    playerHealth_3 = maxHealth_3;
    bossHealth_3 = 100;
    playerScore_3 = 0;
    playerPos_3.x = screenWidth / 100.0f;
    playerPos_3.y = screenHeight - 200;
    groundLevel_3 = playerPos_3.y;

    for (int i = 0; i < MAX_BULLETS_3; i++) {
        bullets_3[i].active_3 = false;
    }

    for (int i = 0; i < MAX_BOSS_BULLETS_3; i++) {
        bossBullets_3[i].active_3 = false;
    }

    for (int i = 0; i < MAX_UFO_BULLETS_3; i++) {
        ufoBullets_3[i].active_3 = false;
    }

    bossBulletTimer_3 = 0.0f;
    bossBulletCount_3 = 0;
    gameState_3 = GAME_STATE_PLAYING_3;
    InitAllUFOs_3();
    isAimingUp_3 = false;
}


void DrawReloadStatus_3(void)
{
    if (isReloading_3) {
        DrawText(TextFormat("RELOADING... %.1f", reloadTimer_3), 10, 100, 20, RED);
    }
    else {
        DrawText(TextFormat("AMMO: %d/2", 2 - bulletsShot_3), 10, 100, 20, BLACK);
    }
}



int main_3() {
    InitWindow(screenWidth, screenHeight, "Boss Battle Game");
    InitTextures_3();
    InitAllUFOs_3();  
    SetTargetFPS(60);
    int count_3 = 0;

    while (!WindowShouldClose()) {
        if (gameState_3 == GAME_STATE_PLAYING_3) {
            if (ufoDefeatedCount_3 >= 3) {
                gameState_3 = GAME_STATE_BOSS_FIGHT_3;
            }
        }

        if (gameState_3 == GAME_STATE_PLAYING_3 || gameState_3 == GAME_STATE_BOSS_FIGHT_3) {
            UpdateMovement_3();
            UpdateJumping_3();
            UpdateReloadSystem_3();


            if (ufoDefeatedCount_3 >= 3 && gameState_3 == GAME_STATE_BOSS_FIGHT_3) {
                bossBulletTimer_3 += GetFrameTime();
                if (bossBulletTimer_3 >= 2.0f && bossBulletCount_3 < MAX_BOSS_BULLET_VOLLEYS_3) {
                    FireBossBullets_3();
                    bossBulletTimer_3 = 0.0f;
                }
            }

            if (IsKeyPressed(KEY_SPACE) && count_3 <= 5) {
                count_3++;
                FireBullet_3(playerPos_3);
            }

            if (isFiring_3) {
                fireTimer_3 -= GetFrameTime();
                if (fireTimer_3 <= 0.9f) {
                    isFiring_3 = false;
                }
            }

            UpdateUFOs_3();  

            if (playerHealth_3 <= 0 || bossHealth_3 <= 0) {
                gameState_3 = GAME_STATE_GAME_OVER_3;
            }
        }
        else if (gameState_3 == GAME_STATE_GAME_OVER_3) {
            if (IsKeyPressed(KEY_R)) {
                ResetGame_3();
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (gameState_3 == GAME_STATE_PLAYING_3 || gameState_3 == GAME_STATE_BOSS_FIGHT_3) {
            DrawTextureEx(background_3, Vector2{ 0, 0 }, 0, 1.0f, WHITE);

            
            for (int i_3 = 0; i_3 < MAX_UFOS_3; i_3++) {
                if (ufos_3[i_3].active_3) {
                    DrawTextureEx(ufo_3, ufos_3[i_3].position_3, 0, 0.3f, WHITE);
                }
            }

            
            if (ufoDefeatedCount_3 >= 3 && gameState_3 == GAME_STATE_BOSS_FIGHT_3) {
                DrawTextureEx(boss_3, bossPos_3, 0, bossScale_3, WHITE);
            }

            DrawPlayer_3();
            UpdateBullets_3();
            UpdateBossBullets_3();
            UpdateUFOBullets();

            DrawHealthBar_3();
            DrawText("Use Left/Right to move, 'J' to Jump, Space to Shoot", 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("UFOs Defeated: %d/3", ufoDefeatedCount_3), 10, 30, 20, DARKGRAY);
        }
        else if (gameState_3 == GAME_STATE_GAME_OVER_3) {
            DrawGameOver_3();
        }

        EndDrawing();
    }

    UnloadTexture(background_3);
    UnloadTexture(player_run1_3);
    UnloadTexture(boss_3);
    UnloadTexture(bulletTexture_3);
    UnloadTexture(boss_bullet_3);

    CloseWindow();
    return 0;
}






int main() {
    int choice;
    do {
        cout << "Menu\n";
        cout << "1.Level 1\n";
        cout << "2.Level 2\n";
        cout << "3.Level 3\n";
        cout << "0.Exit\n";
        cout << "enter your choice :";
        cin >> choice;
        switch (choice) {
        case 1:main1(); break;
        case 2: main2(); break;
       case 3:main_3(); break;
        case 0:cout << "Exit\n"; break;
        default:cout << "invalid choice :"; break;

        }
    } while (choice != 0);


    return 0;
}