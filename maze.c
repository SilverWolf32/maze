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
#include <signal.h>

char *mazeImmutable =
"#########\n"
"#[ #   ]#\n"
"##   ####\n"
"#### ####\n";
char *maze = NULL;

#define STARTCHAR '['
#define EXITCHAR ']'

#define WALLUP (posy <= 0 || maze[(posy-1)*(mazeWidth+1)+posx] != ' ')
#define WALLDOWN (posy >= mazeHeight-1 || maze[(posy+1)*(mazeWidth+1)+posx] != ' ')
#define WALLLEFT (posx <= 0 || maze[(posy*(mazeWidth+1)+posx)-1] != ' ')
#define WALLRIGHT (posx >= mazeWidth-1 || maze[(posy*(mazeWidth+1)+posx)+1] != ' ')

#define EXITUP (!(posy <= 0) && (posy-1)*(mazeWidth+1)+posx == exitpos)
#define EXITDOWN (!(posy >= mazeHeight-1) && (posy+1)*(mazeWidth+1)+posx == exitpos)
#define EXITLEFT (!(posx <= 0) && (posy*(mazeWidth+1)+posx)-1 == exitpos)
#define EXITRIGHT (!(posx >= mazeWidth-1) && (posy*(mazeWidth+1)+posx)+1 == exitpos)

int cols = 0, lines = 0;
int drawOffsetX = 0, drawOffsetY = 0;
int posx = 0, posy = 0, rotation = 1;
int mazeWidth = 0, mazeHeight = 0;
int startpos = 0, exitpos = 0;
bool wallL = 0, wallF = 0, wallR = 0, wallB = 0, exitL = 0, exitF = 0, exitR = 0;

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
void retreat (void) {
	if (wallB) {
		return;
	}
	switch (rotation) {
		case 0:
			posy++;
			break;
		case 1:
			posx--;
			break;
		case 2:
			posy--;
			break;
		case 3:
			posx++;
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
	int absPos = posy*(mazeWidth+1) + posx;
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
			if (WALLDOWN) {
				wallB = 1;
			}
			if (EXITLEFT) {
				exitL = 1;
			}
			if (EXITUP) {
				exitF = 1;
			}
			if (EXITRIGHT) {
				exitR = 1;
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
			if (WALLLEFT) {
				wallB = 1;
			}
			if (EXITUP) {
				exitL = 1;
			}
			if (EXITRIGHT) {
				exitF = 1;
			}
			if (EXITDOWN) {
				exitR = 1;
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
			if (WALLUP) {
				wallB = 1;
			}
			if (EXITRIGHT) {
				exitL = 1;
			}
			if (EXITDOWN) {
				exitF = 1;
			}
			if (EXITLEFT) {
				exitR = 1;
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
			if (WALLRIGHT) {
				wallB = 1;
			}
			if (EXITDOWN) {
				exitL = 1;
			}
			if (EXITLEFT) {
				exitF = 1;
			}
			if (EXITUP) {
				exitR = 1;
			}
			break;
	}
}

void handleWinch (int signal) {
	cols = COLS;
	lines = LINES;
	if (cols > lines*2) {
		drawOffsetX = (cols - lines*2)/2;
		cols = lines*2;
	} else if (lines > cols*2) {
		drawOffsetY = (lines - cols*2)/2;
		lines = cols*2;
	}
	// fprintf(stderr, "\a");
}

int main (int argc, char **argv) {
	// printf("maze len: %lu\n", strlen(mazeImmutable));
	
	if (argc > 1) {
		char *fpath = argv[1];
		FILE *fp = NULL;
		if (strcmp(fpath, "-") == 0) {
			fp = stdin;
		} else {
			fp = fopen(fpath, "r");
		}
		if (!fp) {
			fprintf(stderr, "maze: can't open file '%s': %s\n", fpath, strerror(errno));
			return 1;
		}
		
		maze = malloc(4096 * sizeof(int));
		int c;
		int i;
		for (i = 0; i < 4096 && (c = getc(fp)) != EOF; i++) {
			maze[i] = (char)c;
		}
		maze[i] = '\0';
		if (fp == stdin) {
			freopen("/dev/tty", "r", stdin); // reopen stdin to get user input
		} else {
			fclose(fp);
		}
	} else {
		maze = malloc((strlen(mazeImmutable)+1)*sizeof(char *));
		for (int i = 0; i <= strlen(mazeImmutable); i++) {
			maze[i] = mazeImmutable[i];
		}
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
	
	cols = COLS; lines = LINES;
	if (cols > lines*2) {
		drawOffsetX = (cols - lines*2)/2;
		cols = lines*2;
	} else if (lines > cols*2) {
		drawOffsetY = (lines - cols*2)/2;
		lines = cols*2;
	}
	
	bsd_signal(SIGWINCH, handleWinch);
	
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
				} else {
					if (j != mazeWidth) {
						endwin();
						printf("maze: must be rectangular\n");
						return 1;
					}
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
		wallL = wallF = wallR = wallB = 0;
		exitL = exitF = exitR = 0;
		
		findWalls();
		
		clear();
		
		{
			int inset = lines/10;
			int insetlevel = 0, recursinset = 0;
			int oldposx = posx, oldposy = posy;
			// bool oldWallL = wallL, oldWallF = wallF, oldWallR = wallR;
			do { // loop, draw walls ahead
				wallL = wallF = wallR = wallB = 0;
				exitL = exitF = exitR = 0;
				findWalls();
				
				if (wallL || exitL) {
					for (int i = 0; i < inset; i++) {
						if (drawOffsetY + recursinset+i >= LINES/2-1) {
							break;
						}
						move(drawOffsetY + recursinset+i, drawOffsetX + recursinset+i);
						printw("\\");
					}
					for (int i = 0; i < inset; i++) {
						if (drawOffsetY + lines-1-recursinset-i <= LINES/2) {
							break;
						}
						move(drawOffsetY + lines-1-recursinset-i, drawOffsetX + recursinset+i);
						printw("/");
					}
					if (exitL) {
						for (int i = 0; i < inset; i++) {
							for (int j = recursinset+i; j < lines-recursinset-i; j++) {
								move(drawOffsetY + j, drawOffsetX + recursinset+i);
								printw("|");
							}
						}
					}
				} else {
					for (int i = 1; i < inset; i++) {
						if (drawOffsetY + recursinset+i >= LINES/2-1) {
							break;
						}
						move(drawOffsetY + recursinset+inset, drawOffsetX + recursinset+i);
						printw("-");
						move(drawOffsetY + lines-1-recursinset-inset, drawOffsetX + recursinset+i);
						printw("-");
					}
				}
				for (int i = recursinset+inset; i < lines-recursinset-inset; i++) {
					move(drawOffsetY + i, drawOffsetX + recursinset+inset);
					printw("|");
				}
				if (wallR || exitR) {
					for (int i = 0; i < inset; i++) {
						if (drawOffsetY + recursinset+i >= LINES/2-1) {
							break;
						}
						move(drawOffsetY + recursinset+i, drawOffsetX + cols-1-recursinset-i);
						printw("/");
					}
					for (int i = 0; i < inset; i++) {
						if (drawOffsetY + lines-1-recursinset-i <= LINES/2) {
							break;
						}
						move(drawOffsetY + lines-1-recursinset-i, drawOffsetX + cols-1-recursinset-i);
						printw("\\");
					}
					if (exitR) {
						for (int i = 0; i < inset; i++) {
							for (int j = recursinset+i; j < lines-recursinset-i; j++) {
								move(drawOffsetY + j, drawOffsetX + cols-1-recursinset-i);
								printw("|");
							}
						}
					}
				} else {
					for (int i = 1; i < inset; i++) {
						if (drawOffsetY + recursinset+i >= LINES/2-1) {
							break;
						}
						move(drawOffsetY + recursinset+inset, drawOffsetX + cols-1-recursinset-i);
						printw("-");
						move(drawOffsetY + lines-1-recursinset-inset, drawOffsetX + cols-1-recursinset-i);
						printw("-");
					}
				}
				for (int i = recursinset+inset; i < lines-recursinset-inset; i++) {
					move(drawOffsetY + i, drawOffsetX + cols-1-recursinset-inset);
					printw("|");
				}
				if (wallF || exitF) {
					int istart = recursinset+inset+1, iend = cols-1-recursinset-inset;
					if (!wallL) {
						istart = recursinset+1;
					}
					if (!wallR) {
						iend = cols-1-recursinset;
					}
					for (int i = istart; i < iend; i++) {
						if (drawOffsetY + recursinset+inset >= LINES/2-1) {
							break;
						}
						move(drawOffsetY + recursinset+inset, drawOffsetX + i);
						printw("-");
						move(drawOffsetY + lines-1-recursinset-inset, drawOffsetX + i);
						printw("-");
					}
					// for (int i = recursinset+inset; i < lines-recursinset-inset; i++) {
						// move(i, recursinset+inset);
						// printw("|");
						// move(i, cols-1-recursinset-inset);
						// printw("|");
					// }
					if (exitF) {
						for (int i = istart; i < iend; i++) {
							for (int j = recursinset+inset+1; j < lines-1-recursinset-inset; j++) {
								move(drawOffsetY + j, drawOffsetX + i);
								printw("|");
							}
						}
						wallF = 1;
					}
				}
				
				// break;
				
				// move(lines-1-insetlevel, 0);
				// printw("Handled walls at %d %d, %s%s%s", posx, posy, ((wallL)?"L":""), ((wallF)?"F":""), ((wallR)?"R":""));
				insetlevel++;
				recursinset += inset;
				advance();
			} while (!wallF && recursinset < (lines/2));
			
			posx = oldposx;
			posy = oldposy;
			// move(lines-1, cols-8);
			// printw("%d %d", posx, posy);
			wallL = wallF = wallR = wallB = 0;
			exitL = exitF = exitR = 0;
			findWalls();
			// move(lines-1-insetlevel, 0);
			// printw("Returned to %d %d, %s%s%s", posx, posy, ((wallL)?"L":""), ((wallF)?"F":""), ((wallR)?"R":""));
			refresh();
		}
		
		/* if (exitL) {
			move(0, 0);
			printw("L");
		}
		if (exitF) {
			move(0, 1);
			printw("F");
		}
		if (exitR) {
			move(0, 2);
			printw("R");
		} */
		// move(1, 0);
		// printw("pos: %d %d rot: %d", posx, posy, rotation);
		// printw("pos: %d %d rot: %d | w: %d h: %d", posx, posy, rotation, mazeWidth, mazeHeight);
		/* {
			move(1, 0);
			int absPos = posy*(mazeWidth+1) + posx;
			printw("pos: %d %d rot: %d | abspos %d exitpos %d", posx, posy, rotation, absPos, exitpos);
		} */
		refresh();
		
		/* {
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
		} */
		
		int absPos = posy*(mazeWidth+1) + posx;
		if (absPos == exitpos) {
			endwin();
			printf("You found the exit!\n");
			return 0;
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
			case 's':
				retreat();
				break;
		}
	}
	
	return 0;
}
