#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //usleep()
//necessary headers
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>



#define Size 40
#define limitOfTank 10

#define UP    0
#define	RIGHT 1
#define DOWN  2
#define LEFT  3

//**************DECLARE VARIABLES********************//
typedef char byte;

typedef struct obj{
	byte x;
	byte y;
	byte dir;
	long id;
	long cid;
	struct  obj *blt; //this is not neccessary
	byte isDisabled;
	byte shape[7][2];
}Object;

typedef struct node{
	Object *p;
	struct node *next;
}Node;

Node *listOfTanks = NULL;
Node *listOfBullets = NULL;//watchout

byte numOfTank = 0;
//for input
static struct termios ori_attr, cur_attr;
//#####################################################
//TANK ALPHA\\Controled by Player.
Object *ALPHA; 
//#####################################################
//controler
byte onQuit = 0;

//**************DECLARE FUNCTIOPNS*******************//
Object* newTank(byte x, byte y, byte dir); //braint
Object* newBullet(byte x, byte y, byte dir, long id); 
void addNode(Node **list, Object *t); //THE **list!!!!!!!!!!!!!!!
Node* removeNode(Node *t, long id, long cid);
void moveTank(Object *t, byte (*pointOfMap)[Size]);
void moveBullet(Object *t, byte (*pointOfMap)[Size]);
void handleALPHA(byte (*pointOfMap)[Size]);
byte changeDir();
void cleanWork(Node *list, char *str);
void testList(Node *list, char *str);
void showMap();
void shapeTank(Object *t);
void projectTank(Object *t, byte (*pointOfMap)[Size]);
void projectBullet(Object *blt, byte (*pointOfMap)[Size]);
//for input
static __inline int tty_reset(void);
static __inline int tty_set(void);
static __inline int kbhit(void);
//___________________________________________________//
int main(void){

	//test Tank;;
	Object *t1 = newTank(8, 8, UP);
	Object *t2 = newTank(14, 14, DOWN);
	Object *t3 = newTank(22, 17, LEFT);
	Object *t4 = newTank(29, 22, RIGHT);
	Object *t5 = newTank(12, 19, RIGHT);
	Object *t7 = newTank(30, 8, RIGHT);
	Object *t8 = newTank(22, 5, RIGHT);
	//just test the remove function;
	listOfTanks = removeNode(listOfTanks, t1->id, t1->cid);
	listOfTanks = removeNode(listOfTanks, t2->id, t2->cid);
	ALPHA= (Object *)malloc(sizeof(Object)); //{20, 20, RIGHT, -1, NULL};
	ALPHA->x = 20;
	ALPHA->y = 20;
	ALPHA->dir = RIGHT;
	ALPHA->id = -1;
	ALPHA->blt = NULL;

	//end test;;
	//a useful function
	srand((unsigned)time(NULL));
	//showMap();//####fixed
	while(!onQuit){
		showMap();
		//usleep(150000);//normal speed
		usleep(50000);
		system("clear");
	}
	//
	system("reset");
	testList(listOfTanks, "Tank");
	testList(listOfBullets, "Bullet");
	//clean works
	cleanWork(listOfTanks, "Tank");
	cleanWork(listOfBullets, "Bullet");
	free(ALPHA); //neccessary
	return 0;
}
//**************IMPLEMENTIONS************************//
Object* newTank(byte x, byte y, byte dir){
	Object *pointOfTank = (Object *)malloc(sizeof(Object));
	if(numOfTank >= limitOfTank){
		printf("Tanks are too many!!\n");
		return NULL;
	}
	pointOfTank->x = x;
	pointOfTank->y = y;
	pointOfTank->dir = dir;
	pointOfTank->id = (int)pointOfTank;//warring!!!
	pointOfTank->cid = (int)pointOfTank;//warring!!!
	//###fixed
	addNode(&listOfTanks, pointOfTank);
	return pointOfTank;
}

Object* newBullet(byte x, byte y, byte dir, long id){
	Object *blt = (Object *)malloc(sizeof(Object));
	byte _x, _y;
	switch(dir){
		case UP: _x = x; _y = y - 2; break;
		case DOWN: _x = x; _y = y + 2; break;
		case LEFT: _y = y; _x = x - 2; break;
		case RIGHT: _y = y; _x = x + 2; break;
	}
	blt->x = _x;
	blt->y = _y;
	blt->dir = dir;
	blt->id = id;
	blt->cid = (int)blt; //warring
	return blt;
}

void addNode(Node **list, Object *t){

	Node *node = (Node *)malloc(sizeof(Node));
	if(node == NULL){
        printf("not enough memory");
        exit(-1);
    }
    node->p = t;
    node->next = NULL;

    if(*list == NULL){ //listOfTanks is emepty.###fixed
        *list = node;
    }else{
        if((*list)->next == NULL){ //listOfTanks has only one object.
            (*list)->next = node;
        }else{
            Node *p2node = *list;
            while(p2node->next != NULL){
                p2node = p2node->next;
            }
            p2node->next = node;
        }
    }
    numOfTank += 1;
}

Node* removeNode(Node *t, long id, long cid){
	Node *head = t;
	Node *prev, *nex, *current;

	if((head->p)->id == id){//the element will be removed is the HEAD!
		if(head->next == NULL){//and head is the only one element, 
			free(head);
			return NULL;
		}
		nex = head->next;
		free(head);
		return nex;
	}

	prev = head;
	current = head->next;
	while(current != NULL){// the element will be ??
		nex = current->next;
		if((current->p)->id == id && (current->p)->cid == cid){
			prev->next = nex;
			free(current);
			return head;
		}
		prev = current;
		current = current->next;
	}
}

void moveTank(Object *t, byte (*pointOfMap)[Size]){
	byte x = t->x, y = t->y, dir = t->dir;
	byte min = 2, max = Size - 3;
	byte isTouched;
	byte i = 0, _x, _y;
	//shapeTank(t);
	for(; i < 7; i++){
		_x = t->shape[i][0];
		_y = t->shape[i][1];
		if(pointOfMap[_x][_y] == 3){
			listOfTanks = removeNode(listOfTanks, t->id, t->cid); return;
		}
	}
	if(dir == UP){
		//????????????????????????????????????????????????????????????????????????????????????????
		isTouched = pointOfMap[x][y-2] == 1 || pointOfMap[x-2][y] == 1 || pointOfMap[x+2][y] == 1;
		if(!isTouched) y -= 1;
		else{
			dir = changeDir();
		}
		if(y < min){
			y = min;
			dir = changeDir();//RIGHT;
		}
	}else if(dir == DOWN){
		isTouched = pointOfMap[x][y+2] == 1 || pointOfMap[x+2][y] == 1 || pointOfMap[x-2][y] == 1;
		if(!isTouched) y += 1;
		else{
			dir = changeDir();
		}
		if(y > max){
			y = max;
			dir = changeDir();//LEFT;
		}
	}else if(dir == LEFT){
		isTouched = pointOfMap[x-2][y] == 1 || pointOfMap[x][y+2] == 1 || pointOfMap[x][y-2] == 1;
		if(!isTouched) x -= 1;
		else{
			dir = changeDir();
		}
		if(x < min){
			x = min;
			dir = changeDir();//UP;
		}
	}else{
		isTouched = pointOfMap[x+2][y] == 1 || pointOfMap[x][y-2] == 1 || pointOfMap[x][y+2] == 1;
		if(!isTouched) x += 1;
		else{
			dir = changeDir();
		}
		if(x > max){
			x = max;
			dir = changeDir();//DOWN;
		}
	}
	t->x = x;
	t->y = y;
	t->dir = dir;
	if(rand() % 30 == 1){
		t->blt = newBullet(t->x, t->y, t->dir, t->id);
		addNode(&listOfBullets, t->blt);
	}
}

void moveBullet(Object *t, byte (*pointOfMap)[Size]){
	byte x = t->x, y = t->y, dir = t->dir;
	byte min = 1, max = Size - 2;
	byte isHit = pointOfMap[x][y] == 1;
	if(isHit){
		listOfBullets = removeNode(listOfBullets, t->id, t->cid);
	}
	switch(dir){
		case UP:
			y -= 1;
			if(y < min){
				listOfBullets = removeNode(listOfBullets, t->id, t->cid);
			}
			break;
		case DOWN:
			y += 1;
			if(y > max){
				listOfBullets = removeNode(listOfBullets, t->id, t->cid);
			}
			break;
		case LEFT:
			x -= 1;
			if(x < min){
				listOfBullets = removeNode(listOfBullets, t->id, t->cid);
			}
			break;
		case RIGHT:
			x += 1;
			if(x > max){
				listOfBullets = removeNode(listOfBullets, t->id, t->cid);
			}
			break;
	}
	t->x = x;
	t->y = y;
}

void handleALPHA(byte (*p2map)[Size]){

	byte i, x, y, _x, _y, dir, isTouched;
	byte min = 2, max = Size - 3;
	Node *head;
	//special for ALPHA
    if(ALPHA != NULL){
    	projectTank(ALPHA, p2map);
    	x = ALPHA->x; 
	    y = ALPHA->y;
	    dir = ALPHA->dir;

	    for(i = 0; i < 7; i++){
			_x = ALPHA->shape[i][0];
			_y = ALPHA->shape[i][1];

			head = listOfBullets; //NOTICE HERE###fixed
		    while( head != NULL){
		      	if(_x == (head->p)->x && _y == (head->p)->y){
					free(ALPHA);
					ALPHA = NULL;
					return;
			}
		        head = head->next;
		    } 
		}

	}
    //update ALPHA accord to Player's input
    int tty_set_flag;
    tty_set_flag = tty_set();
	if(kbhit()) {
        const int key = getchar();
        //printf("%c pressed\n", key);
        if(key == 'q'){
        	onQuit = 1;
        	return;
        }
        if(ALPHA != NULL){
	        switch(key){
	        	case 'w':
	        		isTouched = p2map[x][y-2] == 1 || p2map[x-2][y] == 1 || p2map[x+2][y] == 1;
					if(!isTouched) y -= 1;
					if(y < min) y = min;
	        		dir = UP;
	        		break;
	        	case 's': 
	        		
	        		isTouched = p2map[x][y+2] == 1 || p2map[x+2][y] == 1 || p2map[x-2][y] == 1;
					if(!isTouched) y += 1;
					if(y > max) y = max; 
					dir = DOWN;
	        		break;
	        	case 'a': 
	        		
	        		isTouched = p2map[x-2][y] == 1 || p2map[x][y+2] == 1 || p2map[x][y-2] == 1;
					if(!isTouched) x -= 1;
					if(x < min) x = min;
					dir = LEFT;
	        		break;
	        	case 'd':
	        		
	        		isTouched = p2map[x+2][y] == 1 || p2map[x][y-2] == 1 || p2map[x][y+2] == 1;
					if(!isTouched) x += 1;
					if(x > max) x = max; 
					dir = RIGHT;
	        		break;
	        	case 'l':
	        		ALPHA->blt = newBullet(x, y, ALPHA->dir, ALPHA->id);
	        		addNode(&listOfBullets, ALPHA->blt);
	        		break;
	       	}
		    //update copy to ALPHA
			ALPHA->x = x;
			ALPHA->y = y;
			ALPHA->dir = dir;
		}
	}
	//check input setting
    if(tty_set_flag == 0) tty_reset();
}

byte changeDir(){
	//srand((unsigned)time(0));
	return rand() % 4;
}

void cleanWork(Node *list, char *str){
    Node *head = list; //NOTICE HERE###fixed
	Node *next;
    while( head != NULL){
        printf(">%s< @ %p Cleand.\n", str, head->p);
        free(head->p);
        next = head->next;
        printf("<NodeOf%s> @ %p Cleand.\n", str, head);
        free(head);
        head = next;
    } 
    putchar('\n'); 
}

//??????????
void testList(Node *list, char *str){
    int i = 0;
    Node *p2n = list;
    while( p2n != NULL){
        printf("%d>>%s::X=%d, Y=%d, DIR=%d, ID = %d, CID = %d @ %p\n", ++i, str, (p2n->p)->x, (p2n->p)->y, (p2n->p)->dir, (p2n->p)->id, (p2n->p)->cid, p2n->p);
        p2n = p2n->next;
    } 
    putchar('\n');
}



void showMap(void){  //this function should render all the objects. 
	byte map[Size][Size] = {0};
	byte (*p2map)[Size] = map;
	int i, j;
	
	handleALPHA(p2map); ///???????????????????????????????????

	//make the walls
	for(i = 0; i < Size; i++) map[i][0] = 2;
	for(i = 0; i < Size; i++) map[0][i] = 2;
	for(i = 0; i < Size; i++) map[i][Size - 1] = 2;
	for(i = 0; i < Size; i++) map[Size - 1][i] = 2;
	//for each tank
	Node *p2n = listOfTanks;
    while( p2n != NULL){//###cheack itself is hit or not in this loop
    	projectTank(p2n->p, p2map);
        p2n = p2n->next;
    } 
    //for each bullet.
    p2n = listOfBullets;
    while( p2n != NULL){
    	projectBullet(p2n->p, p2map);
        p2n = p2n->next;
    } 
    
    //for each pixel
   	Node *pts = listOfTanks;
   	Node *pbs = listOfBullets;
	for(i = 0; i < Size; i++){
		for(j = 0; j < Size; j ++){
			if(map[j][i] == 0){
				putchar(' '); putchar(' '); //0 for blanks
				//printf("\033[47;37m  \033[0m");
			}else if(map[j][i] == 1){
				putchar('*'); putchar(' '); //1 for Tanks
				//printf("\033[46;36m  \033[0m");
			}else if(map[j][i] == 2){
				putchar('#'); putchar(' '); //2 for walls
				//printf("\033[43;33m  \033[0m");
			}else{
				putchar('+'); putchar(' '); //3 for Bullets
				//printf("\033[41;31m  \033[0m");
			}
		}
	    if( pts != NULL){
	        printf(" %d>>TANK::X = %2d, Y = %2d, DIR = %d, ID = %d @ %p", i, (pts->p)->x, (pts->p)->y, (pts->p)->dir, (pts->p)->id, pts->p);
	        pts = pts->next;

	    } 
	    if(i >= 8){
	    	if( pbs != NULL){
		        printf(" %d>>Bullet::X = %2d, Y = %2d, DIR = %d, ID = %d @ %p", i - 8, (pbs->p)->x, (pbs->p)->y, (pbs->p)->dir, (pbs->p)->id, pbs->p);
		        pbs = pbs->next;
	    	} 
	    }
		putchar('\n');
	}
	
	handleALPHA(p2map); ///???????????????????????????????????
	//update uncontroled tanks
	p2n = listOfTanks;
	while( p2n != NULL){
    	moveTank(p2n->p,  p2map);
        p2n = p2n->next;
    }
    //update each bullet that is abled
    p2n = listOfBullets;
	while( p2n != NULL){
    	moveBullet(p2n->p,  p2map);
        p2n = p2n->next;
    }
    //ending this   
}

void shapeTank(Object *t){
	byte x = t->x, y = t->y, dir = t->dir;

	t->shape[0][0] = x;
	t->shape[0][1] = y;

	if(dir == UP){

		t->shape[1][0] = x;
		t->shape[1][1] = y - 1;

		t->shape[2][0] = x + 1;
		t->shape[2][1] = y;

		t->shape[3][0] = x + 1;
		t->shape[3][1] = y + 1;
	
		t->shape[4][0] = x;
		t->shape[4][1] = y + 1;

		t->shape[5][0] = x - 1;
		t->shape[5][1] = y + 1;

		t->shape[6][0] = x - 1;
		t->shape[6][1] = y;

	}else if(dir == RIGHT){

		t->shape[1][0] = x + 1;
		t->shape[1][1] = y;

		t->shape[2][0] = x;
		t->shape[2][1] = y + 1;

		t->shape[3][0] = x - 1;
		t->shape[3][1] = y + 1;
	
		t->shape[4][0] = x - 1;
		t->shape[4][1] = y;

		t->shape[5][0] = x - 1;
		t->shape[5][1] = y - 1;

		t->shape[6][0] = x;
		t->shape[6][1] = y - 1;

	}else if(dir == DOWN){

		t->shape[1][0] = x;
		t->shape[1][1] = y + 1;

		t->shape[2][0] = x - 1;
		t->shape[2][1] = y;

		t->shape[3][0] = x - 1;
		t->shape[3][1] = y - 1;
	
		t->shape[4][0] = x;
		t->shape[4][1] = y - 1;

		t->shape[5][0] = x + 1;
		t->shape[5][1] = y - 1;

		t->shape[6][0] = x + 1;
		t->shape[6][1] = y;

	}else if(dir == LEFT){

		t->shape[1][0] = x - 1;
		t->shape[1][1] = y;

		t->shape[2][0] = x;
		t->shape[2][1] = y - 1;

		t->shape[3][0] = x + 1;
		t->shape[3][1] = y - 1;
	
		t->shape[4][0] = x + 1;
		t->shape[4][1] = y;

		t->shape[5][0] = x + 1;
		t->shape[5][1] = y + 1;

		t->shape[6][0] = x;
		t->shape[6][1] = y + 1;

	}else{

		t->shape[1][0] = x;
		t->shape[1][1] = y - 1;

		t->shape[2][0] = x + 1;
		t->shape[2][1] = y;

		t->shape[3][0] = x + 1;
		t->shape[3][1] = y + 1;
	
		t->shape[4][0] = x;
		t->shape[4][1] = y + 1;

		t->shape[5][0] = x - 1;
		t->shape[5][1] = y + 1;

		t->shape[6][0] = x - 1;
		t->shape[6][1] = y;

	}
}

void projectTank(Object *t, byte (*pointOfMap)[Size]){
	int i = 0, x, y;
	shapeTank(t);
	for(; i < 7; i++){
		x = t->shape[i][0];
		y = t->shape[i][1];
		pointOfMap[x][y] = 1; //1 :: a block of tank pixel.
	}
}

void projectBullet(Object *blt, byte (*pointOfMap)[Size]){
	pointOfMap[blt->x][blt->y] = 3;
}

//for input
static __inline int tty_reset(void){

    if (tcsetattr(STDIN_FILENO, TCSANOW, &ori_attr) != 0) return -1;
    return 0;
}
static __inline int tty_set(void){
    
    if (tcgetattr(STDIN_FILENO, &ori_attr)) return -1;
    
    memcpy(&cur_attr, &ori_attr, sizeof(cur_attr) );
    cur_attr.c_lflag &= ~ICANON;
	//cur_attr.c_lflag |= ECHO;
    cur_attr.c_lflag &= ~ECHO;
    cur_attr.c_cc[VMIN] = 1;
    cur_attr.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr) != 0) return -1;

    return 0;
}
static __inline int kbhit(void){
                   
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    /* Wait up to five seconds. */
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    retval = select(1, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */

    if (retval == -1) {
        perror("select()");
        return 0;
    } else if (retval)
        return 1;
    /* FD_ISSET(0, &rfds) will be true. */
    else
        return 0;
    return 0;
}
