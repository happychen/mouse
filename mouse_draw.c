#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
//#include "common.h"
#include "fbtools.h"
#include <stdlib.h>

typedef unsigned long int u32_t;
typedef unsigned long int u16_t;
typedef unsigned long int u8_t;
typedef signed long int s8_t;

typedef struct
{
    int dx;
    int dy;
    int dz;
    int button;
}mevent_t;

extern fbscr_t fb_v;

extern int mouse_open(const char *mdev);
extern int mouse_parse(int fd, mevent_t *mevent);

extern int mouse_draw(PFBDEV pFbdev, int x, int y);
extern int mouse_restore(PFBDEV pFbdev, int x, int y);

#define C_WIDTH 10
#define C_HEIGHT 17

#define T___    0xffffffff
#define BORD    0X0
#define X___    0xffff



static u32_t cursor_pixel[C_WIDTH*C_HEIGHT]=
{
    BORD,T___,T___,T___,T___,T___,T___,T___,T___,T___,
    BORD,BORD,T___,T___,T___,T___,T___,T___,T___,T___,
    BORD,X___,BORD,T___,T___,T___,T___,T___,T___,T___,
    BORD,X___,X___,BORD,T___,T___,T___,T___,T___,T___,
    BORD,X___,X___,X___,BORD,T___,T___,T___,T___,T___,

    BORD,X___,X___,X___,X___,BORD,T___,T___,T___,T___,
    BORD,X___,X___,X___,X___,X___,BORD,T___,T___,T___,
    BORD,X___,X___,X___,X___,X___,X___,BORD,T___,T___,
    BORD,X___,X___,X___,X___,X___,X___,X___,BORD,T___,
    BORD,X___,X___,X___,X___,X___,X___,X___,X___,BORD,

    BORD,X___,X___,X___,X___,X___,BORD,BORD,BORD,BORD,
    BORD,X___,X___,BORD,X___,X___,BORD,T___,T___,T___,
    BORD,X___,BORD,T___,BORD,X___,X___,BORD,T___,T___,
    BORD,BORD,T___,T___,BORD,X___,X___,BORD,T___,T___,
    T___,T___,T___,T___,T___,BORD,X___,X___,BORD,T___, 
    T___,T___,T___,T___,T___,BORD,X___,X___,BORD,T___,
    T___,T___,T___,T___,T___,T___,BORD,BORD,T___,T___
};

static u32_t save_cursor[C_HEIGHT*C_WIDTH];

int mouse_test(PFBDEV pFbdev)
{
    int fd;
    if ((fd = mouse_open("/dev/input/mice")) < 0) 
    {
        fprintf(stderr, "Error mouse open:%s\n", strerror(errno));
        exit(1);
    }
    mevent_t mevent;

    int m_x = fb_v.w/2;
    int m_y = fb_v.h/2;
    mouse_draw(pFbdev, m_x, m_y);

    u8_t buf[] = {0xF3, 0xC8, 0xF3, 0x64, 0xF3, 0x50};
    if (write(fd, buf, sizeof(buf)) < sizeof(buf)) 
    {
        fprintf(stderr, "Error write to mice devie:%s\n", strerror(errno));
        fprintf(stderr, "鼠标不支持滚轮\n");
    }

    while(1)
    {
        if (mouse_parse(fd, &mevent) == 0) 
        {
            printf("dx= %d\tdy=%d\tdz=%d\t", mevent.dx, mevent.dy, mevent.dz);
            mouse_restore(pFbdev, m_x, m_y);

            m_x += mevent.dx;
            m_y += mevent.dy;

            mouse_draw(pFbdev, m_x, m_y);
            printf("mx= %d\tmy=%d\n", m_x, m_y);

            switch(mevent.button)
            {
                case 1 :
                        printf("letf button\n");
                        break;
                case 2 :
                        printf("right button\n");
                        break;
                case 4 :
                        printf("middle button\n");
                        break;
                case 0 :
                        printf("no button\n");
                        break;
                default :
                        break;
            }
        }
        else
            ;

    }
    close(fd);
    return 0;
}

int mouse_open(const char *mdev)
{
    if (mdev == NULL) 
    {
        mdev = "/dev/input/mice";
    }
    return (open(mdev, O_RDWR | O_NONBLOCK));
}

#define READ_MOUSE 8
int mouse_parse(int fd, mevent_t *mevent)
{
    s8_t buf[READ_MOUSE];
    int n;
    if ((n = read(fd, buf, READ_MOUSE)) > 0) 
    {
        mevent->dx = buf[1];
        mevent->dy = -buf[2];
        mevent->dz = buf[3];
    }
    else
        return -1;
    
    return 0;
}

 int mouse_save(PFBDEV pFbdev, int x, int y)
{
    int i, j;

    for (j = 0; j < C_HEIGHT; ++j) 
    {
        for (i = 0; i < C_WIDTH; ++i) 
        {
            save_cursor[i + j*C_WIDTH] =
                *(u32_t *)(pFbdev->fb_mem + ((x+i) + (y+j)*fb_v.w)*fb_v.bpp/8);
        }
    }
    return 0;
}

int mouse_draw(PFBDEV pFbdev, int x, int y)
{
    int i, j;
    mouse_save(pFbdev, x, y);
    for (j = 0; j < C_HEIGHT; ++j) 
    {
        for (i = 0; i < C_WIDTH; ++i) 
        {
            if (cursor_pixel[i+j*C_WIDTH] != T___) 
            {
                fb_drawpixel(pFbdev, x+i, y+j, &cursor_pixel[i+j*C_WIDTH]);
            }
        }
    }
    return 0;
}

int mouse_restore(PFBDEV pFbdev, int x, int y)
{
    int i, j;
    for (j = 0; j < C_HEIGHT; ++j) 
    {
        for (i = 0; i < C_WIDTH; ++i) 
        {
            fb_drawpixel(pFbdev, x+i, y+j, &save_cursor[i+j*C_WIDTH]);
        }
    }
    return 0;
}
