#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CLR_SCR printf("\e[1;1H\e[2J")
// #define LINUX
#ifdef LINUX
#include <termios.h>
#include <fcntl.h>
int kbhit();
#else
#include <conio.h>
#include <windows.h>
#endif

// matrix coordinate or simply matrix index/Pos
typedef struct pos
{
    int x;
    int y;
    struct pos *next;
    struct pos *sortNext;
    struct pos *previous;
} Pos;
typedef struct snake
{
    int level;
    int score;
    int size;
    char direction;
    char gameOver;
    Pos *head;
    Pos *tail;
    Pos *sortHead;
} Snake;
typedef struct playground
{
    int width;
    int height;
    int startX;
    int startY;
} Playground;
typedef struct xy
{
    int x;
    int y;
} Fruit;
Pos *createPos(int x, int y);
void setup(Snake *snake, Playground pg);
void draw(Snake s, Playground pg, Fruit fruit);
int isBorder(Playground box, int clientRow, int clientCol);
int isInBox(Playground box, int clientRow, int clientCol);
void clearBox(Playground box);

void input(Snake *s);
int tryingToFlipSnake(Snake *s, int newDirection);
void run(Snake *s, Playground pg, int eaten);
void insertNodeInSortedLinkList(Pos **head, Pos *node);
Fruit generateFruit(Playground);
int didSnakeEatTheFruit(Snake *, Fruit fruit);
void selfCollapse(Snake*);
int main()
{
    Snake snake;
    snake.level = 1;
    snake.direction = 1;
    snake.score = 0;
    Playground playground = {20, 20, 2, 2};
    Playground innerPlayground = {playground.width - 2, playground.height - 2, playground.startX + 1, playground.startY + 1};
    Fruit fruit = generateFruit(innerPlayground);
    // printf("%d %d %d %d",playground.height,playground.width,playground.startX,playground.startY);
    setup(&snake, playground);
    draw(snake, playground, fruit);

    int eaten = 0;
    while (1)
    {   
        input(&snake);
        // printf("\e[?25l");
        draw(snake, playground, fruit);
        // printf("D%d", snake.direction);
        usleep(400000 - snake.score*1000);
        // Sleep(300);
        clearBox(innerPlayground);
        run(&snake, innerPlayground, eaten);
        selfCollapse(&snake);
        if(eaten)
            eaten = 0;
        eaten = didSnakeEatTheFruit(&snake, fruit);
        if (eaten){
            fruit = generateFruit(innerPlayground);
        }
        if (snake.gameOver == 1)
            break;
        // printf("\e[?25h");
    }
    // printf("%d %d",fruit.x,fruit.y);
    // printf("%d",rand()%innerPlayground.height);

    return 0;
}
Pos *createPos(int x, int y)
{
    Pos *posNodePointer = (Pos *)malloc(sizeof(Pos));
    posNodePointer->x = x;
    posNodePointer->y = y;
    posNodePointer->sortNext = NULL;
    posNodePointer->next = NULL;
    posNodePointer->previous = NULL;
    return posNodePointer;
}
void setup(Snake *snake, Playground pg)
{
    CLR_SCR;
    snake->sortHead = createPos(pg.height / 2, pg.width / 2);
    snake->head = snake->sortHead;
    snake->tail = snake->sortHead;
}
void draw(Snake s, Playground pg, Fruit fruit)
{
    printf("\e[%d;%dH", 0, 0);
    Pos *temp = s.sortHead;
    // int r = rand();
    // printf("temp: (%p) %p",temp,s->sortHead);
    // draw the playground
    for (int i = 0; i < pg.startX + pg.height; i++)
    {
        for (int j = 0; j < pg.startY + pg.width; j++)
        {
            // if (i < pg.startX && j < pg.startY)
            //     printf(" ");
          
            if (isBorder(pg, i, j))
            {
                if (s.level == 3)
                    printf("# ");
                else
                    printf("= ");
            }
            else if (fruit.x == i - pg.startX && fruit.y == j - pg.startY){
                printf("🍎");
                // printf("Working");

            }
            else if (temp != NULL && temp->x == i - pg.startX && temp->y == j - pg.startY)
            {
                if (temp == s.head)
                    printf("⊙⊙");
                else
                    printf("00"); 

                // next node
                // if(temp -> sortNext == NULL)
                //     break;
                temp = temp->sortNext;
            }
            else
                printf("  ");
        }
        // if(temp -> sortNext == NULL)
        //             break;
        printf("\n");
    }
    printf("Score: %d\nLevel: %d\n",s.score,s.level);
}
int isBorder(Playground box, int clientRow, int clientCol)
{
    if (isInBox(box, clientRow, clientCol))
    {
        box.startX++;
        box.startY++;
        box.width -= 2;
        box.height -= 2;
        if (isInBox(box, clientRow, clientCol))
            return 0;
        return 1;
    }
    return 0;
}
int isInBox(Playground box, int clientRow, int clientCol)
{
    if ((clientRow >= box.startX && clientRow <= box.startX + box.height - 1) && (clientCol >= box.startY && clientCol <= box.startY + box.width - 1))
        return 1;
    return 0;
}
void clearBox(Playground box)
{
    // move cursor in box at top-left
    //  printf("%d %d",box.startX,box.startY);
    //+1 because [1;1H goes to top-left, and in box i have taken it [0;0]
    //  printf("A");
    //  printf("%d %d",box.height,box.width);
    for (int i = 0; i < box.height; i++)
    {
        printf("\e[%d;%dH", box.startX + 1 + i, box.startY * 2 + 1);
        for (int j = 0; j < box.width; j++)
        {
            printf("  ");
        }
    }
    // printf("\e[%d;%dH",0,0);
}
void input(Snake *s)
{
    if (kbhit())
    {
        char key = getchar();
        //check if it is opposite of current direction
        //if so then return
        switch (key)
        {
        case 'w':
            // up
            if(tryingToFlipSnake(s,2))
                return; 
            s->direction = 0;
            break;
        case 'd':
            // right
            if(tryingToFlipSnake(s,3))
            return;
            s->direction = 1;
            break;
        case 's':
            // down
            if(tryingToFlipSnake(s,0))
            return;
            s->direction = 2;
            break;
        case 'a':
            // left
            if(tryingToFlipSnake(s,1))
            return;
            s->direction = 3;
            break;
        }
    }
}
int tryingToFlipSnake(Snake *s, int newDirection){
    if(s->level!=1 && s->direction == newDirection)
        return 1;
    return 0;
}
void run(Snake *s, Playground pg, int eaten)
{
    // move snake
    // create new head based on current direction
    Pos *posNode;
    int x, y;
    switch (s->direction)
    {
    case 0:
        x = s->head->x - 1;
        if (x == 0)
        {
            if (s->level == 3)
            {
                s->gameOver = 1;
                return;
            }
            else
            {
                x += pg.height;
            }
        }
        posNode = createPos(x, s->head->y);

        break;
    case 1:
        y = s->head->y + 1;
        if (y == pg.width + 1)
        {
            if (s->level == 3)
            {
                s->gameOver = 1;
                return;
            }
            else
            {
                y %= pg.width;
            }
        }
        posNode = createPos(s->head->x, y);
        break;
    case 2:
        x = s->head->x + 1;
        if (x == pg.height + 1)
        {
            if (s->level == 3)
            {
                s->gameOver = 1;
                return;
            }
            else
            {
                x %= pg.height;
            }
        }
        posNode = createPos(x, s->head->y);
        break;
    case 3:
        y = s->head->y - 1;
        if (y == 0)
        {
            if (s->level == 3)
            {
                s->gameOver = 1;
                return;
            }
            else
            {
                y = pg.width + 1;
            }
        }
        posNode = createPos(s->head->x, y);
        break;
    }
    // attach
    posNode->next = s->head;
    s->head->previous = posNode;
    s->head = posNode;
    insertNodeInSortedLinkList(&(s->sortHead), posNode);

    // remove tail
    if (s->level == 1 || !eaten)
    {
        Pos *tail = s->tail;
        s->tail = s->tail->previous;
        s->tail->next = NULL;

        // remove tail node from sort link list
        if (tail == s->sortHead)
        {
            s->sortHead = s->sortHead->sortNext;
        }
        else
        {
            // tail->previous
            Pos *sortPrev, *temp = s->sortHead;
            while (temp != tail)
            {
                sortPrev = temp;
                temp = temp->sortNext;
            }
            sortPrev->sortNext = sortPrev->sortNext->sortNext;
        }
        free(tail);
    }
    // s->tail -> previous -> next = NULL;
}
void insertNodeInSortedLinkList(Pos **head, Pos *node)
{
    Pos *temp = *head, *prev = NULL;
    while (temp != NULL && temp->x <= node->x)
    {
        if (temp->x == node->x)
        {
            if (temp->y > node->y)
            {
                // stop
                break;
            }
        }
        prev = temp;
        temp = temp->sortNext;
    }
    if (prev == NULL)
    {
        // insert before head
        node->sortNext = *head;
        *head = node;
    }
    else
    {
        // insert after prev
        prev->sortNext = node;
        node->sortNext = temp;
    }
}

Fruit generateFruit(Playground pg)
{
    Fruit fruit;
    fruit.x = rand() % pg.height +1;
    fruit.y = rand() % pg.width +1;
    return fruit;
}

int didSnakeEatTheFruit(Snake *s, Fruit fruit)
{
    if (s->head->x == fruit.x && s->head->y == fruit.y)
    {
        s->score += 10;
        if (s->score == 50)
            s->level = 2;
        else if (s->score == 100)
            s->level = 3;

        // extend the tail
        // int x = s->tail->x, y = s->tail Pos *node = createPos() 
        return 1;
    }
    return 0;
}
void selfCollapse(Snake *s){
    Pos *temp = s->head->next;
    while(temp!=NULL){
        if(temp->x == s->head->x && temp->y == s->head->y){
            s->gameOver = 1;
            return;
        }
        temp = temp->next;
    }
    return;
}

#ifdef LINUX
int kbhit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
#endif