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
#include <unistd.h>

char *mazeImmutable =
"#########\n"
"#[ #   ]#\n"
"##   ####\n"
"#### ####\n";
char *maze = NULL;

#define STARTCHAR '['
#define EXITCHAR ']'
// in drawing, don't look at more than this many cells
#define VIEWDEPTH 128

// no fancy arrows
#define ASCII

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
bool showCompass = 0;

bool wallL = 0, wallF = 0, wallR = 0, wallB = 0, exitL = 0, exitF = 0, exitR = 0;

void advance (bool override) {
	if (wallF && !override) {
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
void retreat (bool override) {
	if (wallB && !override) {
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
void moveLeft (bool override) {
	if (wallL && !override) {
		return;
	}
	switch (rotation) {
		case 0:
			posx--;
			break;
		case 1:
			posy--;
			break;
		case 2:
			posx++;
			break;
		case 3:
			posy++;
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
void moveRight (bool override) {
	if (wallR && !override) {
		return;
	}
	switch (rotation) {
		case 0:
			posx++;
			break;
		case 1:
			posy++;
			break;
		case 2:
			posx--;
			break;
		case 3:
			posy--;
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
void _findWalls () {
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
void findWalls (void) {
	wallL = wallF = wallR = wallB = 0;
	exitL = exitF = exitR = 0;
	_findWalls();
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

int _drawWallsPointListIndex = 0;
void _drawWalls (int iter, int horizIter, int ptList[][2]) {
	// move(0+iter, 0);
	// printw("[%d %d]", posx, posy);
	
	for (int i = 0; i < _drawWallsPointListIndex; i++) {
		if (ptList[i][0] == posx && ptList[i][1] == posy) { // been here before
			return;
		}
	}
	
	_drawWallsPointListIndex++;
	ptList[_drawWallsPointListIndex][0] = posx;
	ptList[_drawWallsPointListIndex][1] = posy;
	
	int inset = lines/10;
	int recursinset = iter * inset;
	int horizOffset = horizIter * (cols-1 - ((recursinset+inset) * 2)); // width of front wall
	int oldposx = posx, oldposy = posy;
	// bool oldWallL = wallL, oldWallF = wallF, oldWallR = wallR;
	
	// move(lines-1-iter, 0);
	// for (int i = 0; i < iter; i++) {
	// printw("  ");
	// }
	// printw("Handled walls at %d %d [%s%s%s]", posx, posy, ((wallL)?"L":" "), ((wallF)?"F":" "), ((wallR)?"R":" "));
	// printw(" offset: %d", horizOffset);
	// printw("                    ");
	
	refresh();
	
	findWalls();
	
	if (!wallF && !exitF && recursinset < (lines/2)) {
		// recursinset += inset;
		advance(0);
		_drawWalls(iter+1, horizIter, ptList);
	}
	posx = oldposx;
	posy = oldposy;
	findWalls();
	if (!wallL && !exitL && horizIter <= 0) {
		// advance(0);
		moveLeft(0);
		// printw(" - recursing < to %d %d", posx, posy);
		_drawWalls(iter, horizIter-1, ptList);
	}
	posx = oldposx;
	posy = oldposy;
	findWalls();
	if (!wallR && !exitR && horizIter >= 0) {
		// advance(0);
		moveRight(0);
		// printw(" - recursing > to %d %d", posx, posy);
		_drawWalls(iter, horizIter+1, ptList);
	}
	
	// start of drawing
	
	posx = oldposx;
	posy = oldposy;
	findWalls();
	
	if ((wallL || exitL) && recursinset+inset < LINES/2) {
		for (int i = 0; i < inset; i++) {
			int x = drawOffsetX + recursinset+i + horizOffset;
			if (x < 0 || x > COLS-1) {
				break;
			}
			int y = drawOffsetY + recursinset+i;
			if (y >= LINES/2) {
				break;
			}
			move(y, x);
			printw("\\");
		}
		for (int i = 0; i < inset; i++) {
			int x = drawOffsetX + recursinset+i + horizOffset;
			if (x < 0 || x > COLS-1) {
				break;
			}
			int y = drawOffsetY + lines-1-recursinset-i;
			if (y < LINES/2) {
				break;
			}
			move(y, x);
			printw("/");
		}
		// fill
		for (int i = 0; i < drawOffsetX + inset; i++) {
			int j = recursinset;
			if (i >= drawOffsetX-1) {
				j += i - drawOffsetX + 1;
			}
			int end = lines-j;
			for (; j < end; j++) {
				int x = recursinset+i + horizOffset;
				if (x < 0 || x > COLS-1) {
					break;
				}
				move(drawOffsetY + j, x);
				if (exitL) {
					printw("|");
				} else {
					printw(" ");
				}
			}
		}
	} else {
		
	}
	if (wallL || wallF || exitL || exitF) {
		// draw columns
		for (int i = recursinset+inset; i < lines-recursinset-inset; i++) {
			int x = drawOffsetX + recursinset+inset + horizOffset;
			if (x < 0 || x > COLS-1) {
				break;
			}
			move(drawOffsetY + i, x);
			printw("|");
		}
	}
	if ((wallR || exitR) && recursinset+inset < LINES/2) {
		for (int i = 0; i < inset; i++) {
			int x = drawOffsetX + cols-1-recursinset-i + horizOffset;
			if (x < 0 || x > COLS-1) {
				break;
			}
			int y = drawOffsetY + recursinset+i;
			if (y >= LINES/2) {
				break;
			}
			move(y, x);
			printw("/");
		}
		for (int i = 0; i < inset; i++) {
			int x = drawOffsetX + cols-1-recursinset-i + horizOffset;
			if (x < 0 || x > COLS-1) {
				break;
			}
			int y = drawOffsetY + lines-1-recursinset-i;
			if (y < LINES/2) {
				break;
			}
			move(y, x);
			printw("\\");
		}
		// fill
		for (int i = 0; i < drawOffsetX + inset + 1; i++) {
			int j = recursinset;
			if (i >= drawOffsetX-1) {
				j += i - drawOffsetX + 1;
			}
			int end = lines-j;
			for (; j < end; j++) {
				int x = recursinset+i;
				x = COLS - x - 1;
				x += horizOffset;
				if (x < 0 || x > COLS-1) {
					break;
				}
				move(drawOffsetY + j, x);
				if (exitR) {
					printw("|");
				} else {
					printw(" ");
				}
			}
		}
	} else {
		
	}
	if (wallF || wallR || exitF || exitR) {
		// draw columns
		for (int i = recursinset+inset; i < lines-recursinset-inset; i++) {
			int x = drawOffsetX + cols-1-recursinset-inset + horizOffset;
			if (x < 0 || x > COLS-1) {
				break;
			}
			move(drawOffsetY + i, x);
			printw("|");
		}
	}
	if (wallF || exitF) {
		int istart = recursinset+inset+1, iend = cols-1-recursinset-inset;
		/* if (!wallL) {
			istart = recursinset+1;
		}
		if (!wallR) {
			iend = cols-1-recursinset;
		} */
		for (int i = istart; i < iend; i++) {
			int x = drawOffsetX + i + horizOffset;
			if (x < 0 || x > COLS-1) {
				continue;
			}
			int y = drawOffsetY + recursinset+inset;
			if (y >= LINES/2) {
				break;
			}
			move(drawOffsetY + recursinset+inset, x);
			printw("-");
			move(drawOffsetY + lines-1-recursinset-inset, x);
			printw("-");
		}
		// for (int i = recursinset+inset; i < lines-recursinset-inset; i++) {
		// move(i, recursinset+inset);
		// printw("|");
		// move(i, cols-1-recursinset-inset);
		// printw("|");
		// }
		// fill
		for (int i = istart; i < iend; i++) {
			for (int j = recursinset+inset+1; j < lines-1-recursinset-inset; j++) {
				int x = drawOffsetX + i + horizOffset;
				if (x < 0 || x > COLS-1) {
					continue;
				}
				move(drawOffsetY + j, x);
				if (exitF) {
					printw("|");
				} else {
					printw(" ");
				}
			}
		}
	}
	
	// end of drawing
	
	posx = oldposx;
	posy = oldposy;
	findWalls();
	
	refresh();
}
void drawWalls (void) {
	/* int **ptList = NULL;
	ptList = malloc(VIEWDEPTH * sizeof(int *));
	for (int i = 0; i < VIEWDEPTH; i++) {
		ptList[i] = malloc(2 * sizeof(int));
	} */
	int ptList[VIEWDEPTH][2];
	for (int i = 0; i < VIEWDEPTH; i++) {
		ptList[i][0] = -1; // something impossible
		ptList[i][1] = -1;
	}
	_drawWallsPointListIndex = 0;
	
	_drawWalls(0, 0, ptList);
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
		refresh();
		
		drawWalls();
		
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
			if (wallL) {
				move(0, 0);
				printw("X");
			}
			if (wallF) {
				move(0, 1);
				printw("X");
			}
			if (wallR) {
				move(0, 2);
				printw("X");
			}
			if (exitL) {
				move(1, 0);
				printw("E");
			}
			if (exitF) {
				move(1, 1);
				printw("E");
			}
			if (exitR) {
				move(1, 2);
				printw("E");
			}
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

		// show compass
		if (showCompass) {
			switch (rotation) {
#ifdef ASCII
				case 0: // up
					move(0, 1);
					printw("^");
					move(1, 1);
					printw("|");
					break;
				case 1:
					move(1, 1);
					printw("->");
					break;
				case 2:
					move(1, 1);
					printw("|");
					move(2, 1);
					printw("V");
					break;
				case 3:
					move(1, 0);
					printw("<-");
					break;
#else
				case 0: // up
					move(0, 1);
					printw("▲");
					move(1, 1);
					printw("│");
					break;
				case 1:
					move(1, 1);
					printw("─▶︎");
					break;
				case 2:
					move(1, 1);
					printw("│");
					move(2, 1);
					printw("▼");
					break;
				case 3:
					move(1, 0);
					printw("◀︎─");
					break;
#endif
				default: // should never happen!
					endwin();
					fprintf(stderr, "maze: error: invalid rotation %d (should never happen)\n", rotation);
					exit(2);
					break;
			}
			refresh();
		}
		
		int absPos = posy*(mazeWidth+1) + posx;
		if (absPos == exitpos) {
			endwin();
			printf("You found the exit!\n");
			return 0;
		}
		
		int c = wgetch(stdscr);
		flushinp(); // discard typeahead, prevents holding `?' from locking up program
		switch (c) {
			case 'q': // quit
				;
				char *s1 = "Quit?";
				char *s2 = " [Press Return to quit, Esc to cancel] ";
				int msgwidth = strlen(s2);
				move(LINES/2 - 2, COLS/2 - msgwidth/2);
				int i;
				for (i = 0; i < msgwidth; i++) {
					printw(" ");
				}
				move(LINES/2 - 1, COLS/2 - msgwidth/2);
				for (i = 0; i < msgwidth; i++) {
					printw(" ");
				}
				move(LINES/2 - 1, COLS/2 - strlen(s1)/2);
				printw(s1);
				move(LINES/2, COLS/2 - msgwidth/2);
				printw(s2);
				move(LINES/2 + 1, COLS/2 - msgwidth/2);
				for (i = 0; i < msgwidth; i++) {
					printw(" ");
				}
				
				{
					int c2;
					while ((c2 = wgetch(stdscr)) != '\r' && c2 != '\e' && c2 != ERR) {
						;
					}
					if (c2 == '\r') {
						endwin();
						return 0;
					}
				}
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
				advance(0);
				break;
			case 's':
				retreat(0);
				break;
			case 'C':
				showCompass = !showCompass;
				break;
			case '?':
				// print maze
				move(LINES-mazeHeight, 0);
				int ix = 0, iy = 0;
				for (int i = 0; i < strlen(maze); i++) {
					move(LINES-mazeHeight+iy, ix);
					// int absPos = posy*(mazeWidth+1) + posx;
					if (i == absPos) {
						attron(A_REVERSE);
						printw("*");
						attroff(A_REVERSE);
					} else if (i == startpos) {
						attron(A_REVERSE);
						printw("[");
						attroff(A_REVERSE);
					} else if (i == exitpos) {
						attron(A_REVERSE);
						printw("]");
						attroff(A_REVERSE);
					} else {
						if (maze[i] == '\n') {
							iy++;
							ix = -1;
						} else {
							printw("%c", maze[i]);
						}
					}
					ix++;
				}
				refresh();
				sleep(1);
				break;
			case '':
				refresh();
				break;
		}
	}
	
	return 0;
}
