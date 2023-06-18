#include <curses.h>
#include <stdlib.h>
#include <pthread.h>

/** 向上移动 */
#define UP     1
/** 向下移动 */
#define DOWN  -1
/** 向左移动 */
#define LEFT   2
/** 向右移动 */
#define RIGHT -2

/** 退出 */
#define QUIT_FLAG 'q'

/** 退出标志 */
int g_exit_flag = 1;

/**
 * 蛇的数据结构
 */
struct Snake
{
    /** 行 */
    int row;
    /** 列 */
    int col;
    /** 蛇身子的下一个节点 */
    struct Snake *next;
};

/** 蛇头指针 */
struct Snake *head = NULL;
/** 蛇尾指针 */
struct Snake *tail = NULL;

/** 方向 */
int dir;

/** 食物 */
struct Snake g_food;

/**
 * 初始化食物的位置
 */
void initFood()
{
    int x = rand() % 20;
    int y = rand() % 20;

    g_food.row = x;
    g_food.col = y;
}

/**
 * 初始化 ncurse 库
 */
void initNcurse()
{
    initscr();
    keypad(stdscr, 1);
    // noecho();
}

/**
 * 当前坐标是否有蛇的节点
 */
int hasSnakeNode(int row, int col)
{
    struct Snake *p;
    p = head;
    
    while (p != NULL)
    {
        if (p->row == row && p->col == col)
        {
            return 1;
        }
        
        p = p->next;
    }

    return 0;
}

/**
 * 当前坐标是否有食物的节点
 */
int hasFood(int row, int col)
{
    if (g_food.row == row && g_food.col == col)
    {
        return 1;
    }

    return 0;
}

/**
 * 绘制蛇活动的区域
 */
void gamePic()
{
    int row;
    int col;
    
    move(0, 0);

    for (row = 0; row < 20; row ++)
    {
        if (row == 0)
        {
            for (col = 0; col < 20; col ++)
            {
                printw("--");
            }
            
            printw("\n");
        }
        if (row >= 0 && row <= 19)
        {
            for (col = 0; col <= 20; col ++)
            {
                if (col == 0 || col == 20)
                {
                    printw("|");
                }
                else if (hasSnakeNode(row, col))
                {
                    printw("[]");
                }
                else if (hasFood(row, col))
                {
                    printw("##");
                }
                else
                {
                    printw("  ");
                }
            }
            printw("\n");
        }
        if (row == 19) 
        {
            for (col = 0; col < 20; col ++)
            {
                printw("--");
            }
            printw("\n");
            
            printw("by Netor0x86, g_food.row = %d, g_food.col = %d\n", g_food.row, g_food.col);
        }
    }
}

/**
 * 添加蛇的节点
 */
void addNode()
{
    struct Snake *new  = (struct Snake *)malloc(sizeof(struct Snake));

    new->next = NULL;
    
    switch (dir)
    {
        case UP:
            new->row = tail->row - 1;
            new->col = tail->col;
            break;
        case DOWN:
            new->row = tail->row + 1;
            new->col = tail->col;
            break;
        case LEFT:
            new->row = tail->row;
            new->col = tail->col - 1;
            break;
        case RIGHT:
            new->row = tail->row;
            new->col = tail->col + 1;
            break;
    }

    tail->next = new;

    tail = new;
}

/**
 * 删除蛇身体的节点
 */
void deleteNode()
{
    struct Snake *p;

    p = head;
    head = head->next;

    free(p);
}

/**
 * 初始化蛇
 */
void initSnake()
{
    struct Snake *p;
    
    /* 默认移动方向 */
    dir = RIGHT;
    
    while (head != NULL)
    {
        p = head;
        head = head->next;
        free(p);
    }

    initFood();

    head = (struct Snake *)malloc(sizeof(struct Snake));
    head->row = 1;
    head->col = 1;
    head->next = NULL;

    tail = head;

    addNode();
    addNode();
    addNode();
    addNode();
}

/**
 * 判断蛇是否死掉
 */
int isSnakeDie()
{
    struct Snake *p;
    p = head;

    /* 蛇撞墙 */
    if (tail->row < 0 || tail->col == 0 || tail->row == 20 || tail->col == 20)
    {
        return 1;
    }

    /* 蛇撞到自己 */
    while (p->next != NULL)
    {
        if (p->row == tail->row && p->col == tail->col)
        {
            return 1;
        }

        p = p->next;
    }

    return 0;
}

/**
 * 蛇移动
 */
void moveSnake()
{
    /* 朝移动的方向增加节点 */
    addNode();
    
    /* 吃到食物变长 */
    if (hasFood(tail->row, tail->col))
    {
        initFood();
    }
    else
    {
        /* 没吃到食物，将尾部的节点移除 */
        deleteNode();
    }

    /* 蛇死掉让其回到初始化的位置 */
    if (isSnakeDie())
    {
        initSnake();
    }
}

/**
 * 刷新矩阵
 */
void* refreshMatrix()
{
    while (1)
    {
        moveSnake();

        gamePic();
        
        refresh();
        
        usleep(100000);
    }
}

/**
 * 改变方向的限制
 */
void turn(int direction)
{
    /* 不能直接向相反的方向移动 */
    if (abs(direction) != abs(dir)) {
        dir = direction;
    }
}

/**
 * 处理键盘 - 改变方向 - 退出
 */
void* changeDir()
{
    while (1)    
    {
        int key = getch();
        
        switch (key)
        {
            case KEY_DOWN:
                turn(DOWN);
                break;
            case KEY_UP:
                turn(UP);
                break;
            case KEY_LEFT:
                turn(LEFT);
                break;
            case KEY_RIGHT:
                turn(RIGHT);
                break;
            case QUIT_FLAG:
                g_exit_flag = 0;
                break;
        }
    }
}

int main()
{
    pthread_t t1;
    pthread_t t2;

    initNcurse();
    
    initSnake();

    gamePic();

    pthread_create(&t1, NULL, refreshMatrix, NULL);
    pthread_create(&t2, NULL, changeDir, NULL);

    while (g_exit_flag) ;

    getch();
    endwin();
    return 0;
}
