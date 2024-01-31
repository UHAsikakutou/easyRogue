// Very Simple Rogue-like Game Ver 1.3 for PC-9800 series
// (c)UHAsikakutou 2024
// co-author: ChatGPT (GPT-3.5 Turbo), GitHub Copilot
// MIT License

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <i86.h>

#define MAP_SIZE 10
#define MAX_ENEMIES 5
#define INITIAL_PLAYER_HP 10
#define MAX_ITEMS 2
#define CELEBRATION_MESSAGES 5

enum Command {DUMMY, ATTACK, DEFEND, ESCAPE };

void wait(const unsigned int count)
{
    volatile unsigned int vcount = count;
    while (vcount--);
}

void clearScreen(void) {
    wchar_t __far *a;
    a = (wchar_t __far *)MK_FP(0xa000, 0x0000);
    for (int i = 0; i < 2000; i++) {
        *a = (wchar_t)'\0';
        a++;
    }
    return;
}

void resetGraph(void) {
    char __far *vram_P0_addr;
    char __far *vram_P1_addr;
    char __far *vram_P2_addr;

    vram_P0_addr = (char __far *)MK_FP(0xa800, 0x0000);
    vram_P1_addr = (char __far *)MK_FP(0xb000, 0x0000);
    vram_P2_addr = (char __far *)MK_FP(0xb800, 0x0000);

    // グラフィックス設定
    asm("mov $0x42, %ah");
    asm("mov $0b11000000, %ch");
    asm("int $0x18");

    // グラフィックス使用開始
    asm("mov $0x40, %ah");
    asm("int $0x18");

    for (int y = 0; y < 400;y++){
        for (int x = 0; x < (640/8); x++){
            vram_P0_addr[y * 80 + x] = 0x00;
            vram_P1_addr[y * 80 + x] = 0x00;
            vram_P2_addr[y * 80 + x] = 0x00;
        }
    }
    return;
}

void redScreen(void) {
    char __far *vram_P0_addr;
    char __far *vram_P1_addr;
    char __far *vram_P2_addr;

    vram_P0_addr = (char __far *)MK_FP(0xa800, 0x0000);
    vram_P1_addr = (char __far *)MK_FP(0xb000, 0x0000);
    vram_P2_addr = (char __far *)MK_FP(0xb800, 0x0000);

    // グラフィックス設定
    asm("mov $0x42, %ah");
    asm("mov $0b11000000, %ch");
    asm("int $0x18");

    // グラフィックス使用開始
    asm("mov $0x40, %ah");
    asm("int $0x18");

    for (int y = 0; y < 400;y++){
        for (int x = 0; x < (640/8); x++){
            vram_P0_addr[y * 80 + x] = 0x00;
            vram_P1_addr[y * 80 + x] = 0xff;
            vram_P2_addr[y * 80 + x] = 0x00;
        }
    }
    return;
}

void blueScreen(void) {
    char __far *vram_P0_addr;
    char __far *vram_P1_addr;
    char __far *vram_P2_addr;

    vram_P0_addr = (char __far *)MK_FP(0xa800, 0x0000);
    vram_P1_addr = (char __far *)MK_FP(0xb000, 0x0000);
    vram_P2_addr = (char __far *)MK_FP(0xb800, 0x0000);

    // グラフィックス設定
    asm("mov $0x42, %ah");
    asm("mov $0b11000000, %ch");
    asm("int $0x18");

    // グラフィックス使用開始
    asm("mov $0x40, %ah");
    asm("int $0x18");

    for (int y = 0; y < 400;y++){
        for (int x = 0; x < (640/8); x++){
            vram_P0_addr[y * 80 + x] = 0xff;
            vram_P1_addr[y * 80 + x] = 0x00;
            vram_P2_addr[y * 80 + x] = 0x00;
        }
    }
    return;
}

void greenScreen(void) {
    char __far *vram_P0_addr;
    char __far *vram_P1_addr;
    char __far *vram_P2_addr;

    vram_P0_addr = (char __far *)MK_FP(0xa800, 0x0000);
    vram_P1_addr = (char __far *)MK_FP(0xb000, 0x0000);
    vram_P2_addr = (char __far *)MK_FP(0xb800, 0x0000);

    // グラフィックス設定
    asm("mov $0x42, %ah");
    asm("mov $0b11000000, %ch");
    asm("int $0x18");

    // グラフィックス使用開始
    asm("mov $0x40, %ah");
    asm("int $0x18");

    for (int y = 0; y < 400;y++){
        for (int x = 0; x < (640/8); x++){
            vram_P0_addr[y * 80 + x] = 0x00;
            vram_P1_addr[y * 80 + x] = 0x00;
            vram_P2_addr[y * 80 + x] = 0xff;
        }
    }
    return;
}

void displayBattleScene(int enemyHP, int playerHP) {
    printf("=== Battle Scene ===\n");
    printf("Enemy HP: %d  Player HP: %d \n", enemyHP,playerHP);
    printf("1. Attack\n");
    printf("2. Defend\n");
    printf("3. Escape\n");
}

char map[MAP_SIZE][MAP_SIZE];
int playerX, playerY;
int goalX, goalY;
int enemyX[MAX_ENEMIES], enemyY[MAX_ENEMIES];
int itemX[MAX_ITEMS], itemY[MAX_ITEMS];
int playerHP;
int floorNumber = 1;
int actions = 0;

void showTitleScreen() {
    clearScreen();
    resetGraph();
    printf("This game requires a terminal with a size of at least 32x20.\n");
    printf("If you can't see the title screen correctly, please resize your "
           "terminal.\n");
    printf("Press Enter to continue.\n");
    getchar();

    clearScreen();

    printf("                                \n");
    printf("--------------------------------\n");
    printf("         VERY SIMPLE            \n");
    printf(",---.                           \n");
    printf("|---',---.,---..   .,---.       \n");
    printf("|   ||   ||   ||   ||---'      \n");
    printf("`   ``---'`---|`---'`---'  like \n");
    printf("          `---'                 \n");
    printf("                                \n");
    printf("         PRESS ENTER            \n");
    printf("--------------------------------\n");
    printf("Ver. 1.3                        \n");
    printf("(C)UHAsikakutou 2024            \n");
    printf("co-author:                      \n");
    printf("- ChatGPT (GPT-3.5 Turbo)       \n");
    printf("- GitHub Copilot                \n");
    printf("This game is licensed under the \n");
    printf("MIT License.                    \n");

    // キーが押されるまで待機
    getchar();
}

void placeItems() {
    int numItems =
        rand() % (MAX_ITEMS + 1); // 0からMAX_ITEMS個のアイテムをランダムで配置

    for (int i = 0; i < numItems; ++i) {
        itemX[i] = rand() % MAP_SIZE;
        itemY[i] = rand() % MAP_SIZE;
        while ((itemX[i] == playerX && itemY[i] == playerY) ||
               (itemX[i] == goalX && itemY[i] == goalY) ||
               (itemX[i] == enemyX[0] && itemY[i] == enemyY[0]) ||
               (itemX[i] == enemyX[1] && itemY[i] == enemyY[1]) ||
               (itemX[i] == enemyX[2] && itemY[i] == enemyY[2]) ||
               (itemX[i] == enemyX[3] && itemY[i] == enemyY[3]) ||
               (itemX[i] == enemyX[4] && itemY[i] == enemyY[4])) {
            itemX[i] = rand() % MAP_SIZE;
            itemY[i] = rand() % MAP_SIZE;
        }
        map[itemY[i]][itemX[i]] = 'I';
    }
}

void initializeMap() {
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            map[i][j] = '.';
        }
    }
    placeItems();
}

void placePlayer() {
    playerX = rand() % MAP_SIZE;
    playerY = rand() % MAP_SIZE;
    map[playerY][playerX] = 'P';
    playerHP = INITIAL_PLAYER_HP;
}

void placeGoal() {
    goalX = rand() % MAP_SIZE;
    goalY = rand() % MAP_SIZE;
    while (goalX == playerX && goalY == playerY) {
        goalX = rand() % MAP_SIZE;
        goalY = rand() % MAP_SIZE;
    }
    map[goalY][goalX] = 'G';
}

void collectItem() {
    for (int i = 0; i < MAX_ITEMS; ++i) {
        if (playerX == itemX[i] && playerY == itemY[i]) {
            int healAmount = rand() % 5 + 1; // 1から5のランダムな回復量
            printf("You found an item! HP + %d\n", healAmount);
            playerHP += healAmount;
            if (playerHP > INITIAL_PLAYER_HP) {
                playerHP = INITIAL_PLAYER_HP; // 最大HPを超えないようにする
            }
            map[itemY[i]][itemX[i]] = '.'; // アイテムを回収したらマップから消す
            itemX[i] = -1; // アイテム座標を初期化
            itemY[i] = -1;
        }
    }
}

void placeEnemies() {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemyX[i] = rand() % MAP_SIZE;
        enemyY[i] = rand() % MAP_SIZE;
        while ((enemyX[i] == playerX && enemyY[i] == playerY) ||
               (enemyX[i] == goalX && enemyY[i] == goalY)) {
            enemyX[i] = rand() % MAP_SIZE;
            enemyY[i] = rand() % MAP_SIZE;
        }
        map[enemyY[i]][enemyX[i]] = 'E';
    }
}

void printMap() {
    clearScreen();
    printf("Very Simple Rogue-like Game Ver 1.3\n");
    printf("(c)UHAsikakutou 2024\n\n");
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            printf("%c ", map[i][j]);
        }
        printf("\n");
    }
    printf("\nPlayer HP: %d\n", playerHP);
    printf("Floor: %d, Actions: %d\n", floorNumber, actions);
}

void movePlayer(char direction) {
    int isCorrectDirection = 0;
    map[playerY][playerX] = '.';
    switch (direction) {
    case 'w':
    case 'W':
        if (playerY > 0)
            playerY--;
        break;
    case 'a':
    case 'A':
        if (playerX > 0)
            playerX--;
        break;
    case 's':
    case 'S':
        if (playerY < MAP_SIZE - 1)
            playerY++;
        break;
    case 'd':
    case 'D':
        if (playerX < MAP_SIZE - 1)
            playerX++;
        break;
    default:
        printf("Invalid input. Please use W, A, S, D for movement.\n");
        isCorrectDirection = 1;
        wait(1); // 0.5秒の待ち時間
        return;
    }
    map[playerY][playerX] = 'P';
    actions++;
}

void moveEnemies(int goalX, int goalY) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        int direction = rand() % 4; // 0: up, 1: left, 2: down, 3: right

        int newEnemyX = enemyX[i];
        int newEnemyY = enemyY[i];

        // ゴールの位置を渡し、ゴールに移動しようとした場合は元の位置にとどまる
        if (newEnemyX == goalX && newEnemyY == goalY) {
            continue;
        }

        switch (direction) {
        case 0:
            if (newEnemyY > 0)
                newEnemyY--;
            break;
        case 1:
            if (newEnemyX > 0)
                newEnemyX--;
            break;
        case 2:
            if (newEnemyY < MAP_SIZE - 1)
                newEnemyY++;
            break;
        case 3:
            if (newEnemyX < MAP_SIZE - 1)
                newEnemyX++;
            break;
        }

        // 移動前の位置をクリア
        map[enemyY[i]][enemyX[i]] = '.';

        // 新しい位置をセット
        enemyX[i] = newEnemyX;
        enemyY[i] = newEnemyY;

        // 移動後の位置に敵を表示
        map[enemyY[i]][enemyX[i]] = 'E';
    }
}

void replaceGoal() { map[goalY][goalX] = 'G'; }

void celebrate() {
    const char *celebration_messages[CELEBRATION_MESSAGES] = {
        "Congratulations! You are a Rogue Master!",
        "Hooray! You've conquered another set of floors!",
        "Well done! You're on fire!",
        "Amazing job! Keep up the good work!",
        "You're unstoppable! Keep adventuring!",
    };

    int randomIndex = rand() % CELEBRATION_MESSAGES;
    printf("\n%s\n", celebration_messages[randomIndex]);
}

int processBattle(enum Command playerCommand, int enemyHP) {
    // 仮の戦闘処理
    int isSuccessfullEscape = 0;
    switch (playerCommand) {
    case ATTACK:
        printf("You dealt 3 damage to the enemy!\n");
        enemyHP -= 3;
        printf("The enemy counterattacks! You received 2 damage.\n");
        playerHP -= 2;
        wait(1);
        break;
    case DEFEND:
        printf("You defended against the enemy's attack!\n");
        if (rand() % 4 == 0) {
            printf("The enemy counterattacks! You received 2 damage.\n");
            playerHP -= 2;
        } else {
            printf("The enemy missed its counterattack.\n");
        }
        wait(1);
        break;
    case ESCAPE:
        if (rand() % 2 == 0) {
            printf("You successfully escaped from the enemy!\n");
            isSuccessfullEscape = 1;
            wait(1);
            return 0;
        } else {
            printf("You failed to escape from the enemy!\n");
            printf("The enemy counterattacks! You received 2 damage.\n");
            playerHP -= 2;
            wait(1);
        }
        wait(1);
        break;
    }
    // 敵のHPが0以下になったら戦闘終了
    if (enemyHP <= 0) {
        greenScreen();
        printf("You defeated the enemy!\n");
        wait(1);
        return 0;
    } else if (playerHP <= 0) {
        printf("You were defeated by the enemy!\n");
        wait(1);
        return -1;
    } else if (isSuccessfullEscape) {
        printf("You escaped from the enemy!\n");
        return 0;
    } else {
        return enemyHP;
    }
}

int main() {
    srand(time(NULL));
    showTitleScreen();

    placePlayer();

    while (playerHP > 0) {
        initializeMap();
        placeEnemies();
        placeGoal();
        actions = 0;

        while (!(playerX == goalX && playerY == goalY) && playerHP > 0) {
            printMap();
            collectItem();
            replaceGoal();
            char move;
            printf("Enter your move (WASD): ");
            scanf(" %c", &move);

            moveEnemies(goalX, goalY);
            movePlayer(move);

            // Check if player encounters an enemy
            for (int i = 0; i < MAX_ENEMIES; ++i) {
                if (playerX == enemyX[i] && playerY == enemyY[i]) {
                    printf(
                        "You encountered an enemy! Entering battle scene.\n");
                        blueScreen();
                    // 既存のマップの状態を保存
                    char originalMap[MAP_SIZE][MAP_SIZE];
                    for (int i = 0; i < MAP_SIZE; ++i) {
                        for (int j = 0; j < MAP_SIZE; ++j) {
                            originalMap[i][j] = map[i][j];
                        }
                    }

                    // 戦闘シーンの処理

                    // 戦闘処理を実行
                    int enemyHP = 10;
                    while (1) {
                        enum Command playerCommand;
                        do {
                            displayBattleScene(enemyHP,playerHP);
                            printf("Choose your command (1-3): ");
                            scanf("%d", (int *)&playerCommand);

                            if (playerCommand < ATTACK ||
                                playerCommand > ESCAPE) {
                                printf("Invalid command. Please choose a valid command.\n");
                            }
                        } while (playerCommand < ATTACK ||
                                 playerCommand > ESCAPE);

                        enemyHP = processBattle(playerCommand, enemyHP);
                        if (enemyHP == 0) {
                            break;
                        } else if (enemyHP == -1) {
                            break;
                        }
                    }
                    // 戦闘後、元のマップに復帰
                    for (int i = 0; i < MAP_SIZE; ++i) {
                        for (int j = 0; j < MAP_SIZE; ++j) {
                            map[i][j] = originalMap[i][j];
                        }
                    }
                    break; // 敵とのエンカウント後は戦闘終了
                }
            }
        }

        if (playerHP > 0) {
            printf("Congratulations! You reached the goal and advanced to the next floor.\n");
            floorNumber++;
            if (floorNumber % 5 == 0) {
                celebrate();
            }
            wait(1); // 0.5秒の待ち時間
        } else {
            redScreen();
            printf("Game Over! Your HP reached 0.\n");
            printf("You reached floor %d.\n", floorNumber);
            break;
        }

        // ユーザーの入力待ち
        getchar();
    }

    if (playerHP > 0) {
        printf("Congratulations! You completed all floors.\n");
    }

    return 0;
}
