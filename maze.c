//
// maze.c
// by SilverWolf
//
// A simple ASCII art first-person maze, inspired by IOCCC 'buzzard'
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <curses.h>

char *mazeImmutable =
"========\n"
"=[ =   ]\n"
"==   ===\n"
"==== ===\n";
char *maze = NULL;

#define STARTCHAR '['
#define EXITCHAR ']'

#define WALLUP (posy <= 0 || maze[(posy-1)*(mazeWidth+1)+posx] != ' ')
#define WALLDOWN (posy >= mazeHeight-1 || maze[(posy+1)*(mazeWidth+1)+posx] != ' ')
#define WALLLEFT (posx <= 0 || maze[(posy*(mazeWidth+1)+posx)-1] != ' ')
#define WALLRIGHT (posx >= mazeWidth-1 || maze[(posy*(mazeWidth+1)+posx)+1] != ' ')

int posx = 0, posy = 0, rotation = 1;
int mazeWidth = 0, mazeHeight = 0;
int startpos = 0, exitpos = 0;
bool wallL = 0, wallF = 0, wallR = 0;

void advance (void) {
	if (wallF) {
		return;
	}
	switch (rotation) {
		case 0:
			posy--;
			break;
		case 1:
			posx++;
			break;
		case 2:
			posy++;
			break;
		case 3:
			posx--;
			break;
		default:
			break;
	}
	if (posx < 0) {
		posx = 0;
	}
	if (posx > mazeWidth-1) {
		posx = mazeWidth-1;
	}
	if (posy < 0) {
		posy = 0;
	}
	if (posy > mazeHeight-1) {
		posy = mazeHeight-1;
	}
}
void findWalls (void) {
	// int absPos = posy*(mazeWidth+1) + posx;
	switch (rotation) {
		case 0: // facing up
			if (WALLLEFT) {
				wallL = 1;
			}
			if (WALLUP) {
				wallF = 1;
			}
			if (WALLRIGHT) {
				wallR = 1;
			}
			break;
		case 1: // facing right
			if (WALLUP) {
				wallL = 1;
			}
			if (WALLRIGHT) {
				wallF = 1;
			}
			if (WALLDOWN) {
				wallR = 1;
			}
			break;
		case 2: // facing down
			if (WALLRIGHT) {
				wallL = 1;
			}
			if (WALLDOWN) {
				wallF = 1;
			}
			if (WALLLEFT) {
				wallR = 1;
			}
			break;
		case 3: // facing left
			if (WALLDOWN) {
				wallL = 1;
			}
			if (WALLLEFT) {
				wallF = 1;
			}
			if (WALLUP) {
				wallR = 1;
			}
			break;
	}
}

int main (int argc, char **argv) {
	// printf("maze len: %lu\n", strlen(mazeImmutable));
	
	maze = malloc((strlen(mazeImmutable)+1)*sizeof(char *));
	for (int i = 0; i <= strlen(mazeImmutable); i++) {
		maze[i] = mazeImmutable[i];
	}
	
	setlocale(LC_ALL, "");
	
	initscr();
	cbreak();
	noecho();
	
	nonl();
	
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	set_escdelay(10);
	curs_set(0);
	
	refresh();
	
	// endwin(); // for testing
	// printf("%s\n", mazeImmutable);
	// printf("%s\n", maze);
	
	
	// find position
	{
		int i, j, newlines = 0;
		bool firstNewline = 1;
		for (i = 0, j = 0; i < strlen(maze); i++, j++) {
			if (maze[i] == '\n') {
				if (firstNewline) {
					// printf("%%");
					firstNewline = 0;
					mazeWidth = j;
				}
				// printf("\n");
				newlines++;
				j = -1; // it'll be incremented to 0
			} else if (maze[i] == STARTCHAR) {
				// printf("!");
				posx = j;
				posy = newlines;
				startpos = i;
				maze[i] = ' ';
				// printf("Found start at %d %d\n", posx, posy);
			} else if (maze[i] == EXITCHAR) {
				// printf("!");
				exitpos = i;
				maze[i] = ' ';
			} else {
				// printf(".");
			}
		}
		mazeHeight = newlines;
	}
	
	// endwin();
	// printf("posx: %d posy: %d maze w: %d maze h: %d\n", posx, posy, mazeWidth, mazeHeight);
	// return 0;
	
	while (1) {
		wallL = wallF = wallR = 0;
		
		findWalls();
		
		clear();
		
		{
			int inset = LINES/10;
			int insetlevel = 0, recursinset = 0;
			int oldposx = posx, oldposy = posy;
			// bool oldWallL = wallL, oldWallF = wallF, oldWallR = wallR;
			do { // loop, draw walls ahead
				wallL = wallF = wallR = 0;
				findWalls();
				
				if (wallL) {
					for (int i = 0; i < inset; i++) {
						move(recursinset+i, recursinset+i);
						printw("\\");
					}
					for (int i = 0; i < inset; i++) {
						move(LINES-1-recursinset-i, recursinset+i);
						printw("/");
					}
				} else {
					for (int i = 0; i < inset; i++) {
						move(recursinset+inset, recursinset+i);
						printw("-");
						move(LINES-1-recursinset-inset, recursinset+i);
						printw("-");
					}
				}
				for (int i = recursinset+inset; i < LINES-recursinset-inset; i++) {
					move(i, recursinset+inset);
					printw("|");
				}
				if (wallR) {
					for (int i = 0; i < inset; i++) {
						move(recursinset+i, COLS-1-recursinset-i);
						printw("/");
					}
					for (int i = 0; i < inset; i++) {
						move(LINES-1-recursinset-i, COLS-1-recursinset-i);
						printw("\\");
					}
				} else {
					for (int i = 0; i < inset; i++) {
						move(recursinset+inset, COLS-1-recursinset-i);
						printw("-");
						move(LINES-1-recursinset-inset, COLS-1-recursinset-i);
						printw("-");
					}
				}
				for (int i = recursinset+inset; i < LINES-recursinset-inset; i++) {
					move(i, COLS-1-recursinset-inset);
					printw("|");
				}
				if (wallF) {
					int istart = recursinset+inset+1, iend = COLS-1-recursinset-inset;
					if (!wallL) {
						istart = recursinset;
					}
					if (!wallR) {
						iend = COLS-recursinset;
					}
					for (int i = istart; i < iend; i++) {
						move(recursinset+inset, i);
						printw("-");
						move(LINES-1-recursinset-inset, i);
						printw("-");
					}
					// for (int i = recursinset+inset; i < LINES-recursinset-inset; i++) {
						// move(i, recursinset+inset);
						// printw("|");
						// move(i, COLS-1-recursinset-inset);
						// printw("|");
					// }
				}
				
				// break;
				
				move(LINES-1-insetlevel, 0);
				printw("Handled walls at %d %d, %s%s%s", posx, posy, ((wallL)?"L":""), ((wallF)?"F":""), ((wallR)?"R":""));
				insetlevel++;
				recursinset += inset;
				advance();
			} while (!wallF);
			posx = oldposx;
			posy = oldposy;
			// move(LINES-1, COLS-8);
			// printw("%d %d", posx, posy);
			wallL = wallF = wallR = 0;
			findWalls();
			move(LINES-1-insetlevel, 0);
			printw("Returned to %d %d, %s%s%s", posx, posy, ((wallL)?"L":""), ((wallF)?"F":""), ((wallR)?"R":""));
			refresh();
		}
		
		if (wallL) {
			move(0, 0);
			printw("L");
		}
		if (wallF) {
			move(0, 1);
			printw("F");
		}
		if (wallR) {
			move(0, 2);
			printw("R");
		}
		move(1, 0);
		// printw("pos: %d %d rot: %d", posx, posy, rotation);
		printw("pos: %d %d rot: %d | w: %d h: %d", posx, posy, rotation, mazeWidth, mazeHeight);
		refresh();
		
		{
			if (WALLUP || WALLLEFT) {
				move(2, 0);
				printw("X");
			}
			if (WALLUP) {
				move(2, 1);
				printw("X");
			}
			if (WALLUP || WALLRIGHT) {
				move(2, 2);
				printw("X");
			}
			if (WALLLEFT) {
				move(3, 0);
				printw("X");
			}
			if (WALLRIGHT) {
				move(3, 2);
				printw("X");
			}
			if (WALLDOWN || WALLLEFT) {
				move(4, 0);
				printw("X");
			}
			if (WALLDOWN) {
				move(4, 1);
				printw("X");
			}
			if (WALLDOWN || WALLRIGHT) {
				move(4, 2);
				printw("X");
			}
			// move(6, 0);
			// printw("%s", maze);
			refresh();
		}
		
		int c = wgetch(stdscr);
		switch (c) {
			case 'q':
				endwin();
				return 0;
				break;
			case 'a': // rotate left
				rotation--;
				if (rotation < 0) {
					rotation = 3; // wrap around
				}
				break;
			case 'd': // rotate right
				rotation = (rotation+1)%4;
				break;
			case 'w':
				advance();
				break;
		}
	}
	
	return 0;
}
