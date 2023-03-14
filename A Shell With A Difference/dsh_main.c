#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <time.h>
#include <ncurses.h>
#include <dirent.h>

#include"dsh.h"

#define WIN_HEIGHT 35
#define WIN_WIDTH 50
#define WIN1_START_X 10
#define WIN2_START_X (WIN1_START_X + WIN_WIDTH + 20)
#define WIN_START_Y 10

#define TOT_INPUT 100

int k = 0;
int Nsession = 0;
int N = 0;
void printStat (WINDOW* input, char beta[][100], int count) {	
	mvwprintw(input,4+k,10,"%d :%s",k, beta[k]);
	k++;
	wrefresh(input);
}

void printStat2(WINDOW* output) {
	wclear(output);
	int x = 5;
	int y = 5;
    int checkDir = 0;
    while (!checkDir) {
        char subfolder[100];
        snprintf(subfolder,100,"%s/.dsh/%d",getenv("HOME"), Nsession);
        DIR* dir = opendir(subfolder);
        if (dir) {
            Nsession++;
            closedir(dir);
        } else {
            Nsession--;
            checkDir = 1;
        }
    }
	char session[120];
	snprintf(session, 120, "%s/.dsh/%d/%d.stdout", getenv("HOME"),Nsession, N);
	FILE *src;
	src = fopen(session,"r");
	char ch;
	mvwprintw(output, y-1, x-1,"%d.stdout: ",N);
    while((ch = getc(src)) != EOF) {
		if(ch != '\n') {
			mvwprintw(output, y, x,"%c",ch);
			x++;
		}
		else {
			x = 5;
			y++;
			mvwprintw(output, y, x,"%c",ch);
		}
	}
	wrefresh(output);
	fclose(src);
	char session2[120];
	snprintf(session2, 120, "%s/.dsh/%d/%d.stderr", getenv("HOME"),Nsession, N);
	FILE *src1;
	char ch1;
	src1 = fopen(session2,"r");
	x = 5;
	y = 5;
	mvwprintw(output, y+9, x-1,"%d.stderr: ",N);
    while((ch1 = getc(src1)) != EOF) {
		if(ch1 != '\n') {
			mvwprintw(output, y+10, x,"%c",ch1);
			x++;
		}
		else {
			x = 5;
			y++;
			mvwprintw(output, y+10, x,"%c",ch1);
		}
	}
	wrefresh(output);
	fclose(src1);
	N++;	
}

void keyupHelper(WINDOW* input, char beta[][100], int row, int count) {
	wclear(input);
	int x = 0;
	while(x < row) {
		if(x == count) {
			wattron(input, A_STANDOUT);
			mvwprintw(input,4+count,10,"%d :%s",count, beta[count]);
			wattroff(input,A_STANDOUT);
			x++;
		}
		else {
			mvwprintw(input,4+x,10,"%d :%s",x, beta[x]);
			x++;
		}
	}
	box(input,0,0);
	wrefresh(input);
}

void keydownHelper(WINDOW* input, char beta[][100], int row, int count) {
	wclear(input);
	int x = 0;
	while(x < row) {
		if(x == count) {
			wattron(input, A_STANDOUT);
			mvwprintw(input,4+count,10,"%d :%s",count, beta[count]);
			wattroff(input,A_STANDOUT);
			x++;
		}
		else {
			mvwprintw(input,4+x,10,"%d :%s",x, beta[x]);
			x++;
		}
	}
	box(input,0,0);
	wrefresh(input);
}

int main(int argc, char** argv) {
    /* Initialize the screen */
	initscr();

	/* Make sure that Ctrl+C will exit the program */
	cbreak();

	/* Disable default key press echo */
	noecho();
	
	char *line=0;
    size_t size=0;
	char alpha[TOT_INPUT]={0};
	char temp[TOT_INPUT]={0};
	char beta[TOT_INPUT][TOT_INPUT]={0};

	/* Create input window on the left with heading */
	mvwprintw(stdscr, WIN_START_Y - 2, WIN1_START_X, "%s", "Dsh Window");
	WINDOW *input = newwin(WIN_HEIGHT, WIN_WIDTH, WIN_START_Y, WIN1_START_X);

	mvwprintw(stdscr, WIN_START_Y - 2, WIN2_START_X, "%s", "Output Window");
	WINDOW *output = newwin(WIN_HEIGHT, WIN_WIDTH, WIN_START_Y, WIN2_START_X);
	
	refresh();

	/* Create box around input window */
	box(input,0,0);
	wrefresh(input);
	keypad(input, true);

	box(output,0,0);
	wrefresh(output);
	
	int input_y0 = 1;
	int input_y  = WIN_HEIGHT-2;
	int input_x  = 1;
	int input_x1 = 6;
	int input_y1 = 4;
	
	dsh_init();
	mvwprintw(input,input_y0,input_x,"History");
	mvwprintw(input,input_y,input_x,"%s","dsh> ");
	wmove(input,input_y,input_x1);
	box(input,0,0);
	wrefresh(input);
	
	int inputChar;
	
	int i = 0,count = 0, col = 0, row = 0;

    while(i < 100) {
		inputChar = wgetch(input);
		alpha[i] = (char)inputChar;
		strcpy(temp,alpha);
		beta[row][col] = temp[i];
		if(inputChar == KEY_BACKSPACE) {
			int x = 4 + i + 1;
			if(x > 5) {
				mvwdelch(input,input_y,x);
				wrefresh(input);
				alpha[i] = '\0';
				beta[row][i] = alpha[i];
				i--;
				wmove(input,input_y,x-1);
			}
		}
		else if(inputChar == '\n') {
			dsh_run(alpha);
			printStat(input,beta,count);
			printStat2(output);
			box(output,0,0);
			wrefresh(output);
			wmove(input,input_y,input_x);
			wclrtoeol(input);
			mvwprintw(input,input_y,input_x,"%s","dsh> ");
			wmove(input,input_y,input_x1);
			box(input,0,0);
			wrefresh(input);
			for(int j = 0; j < 100; j++) {
				alpha[j] = '\0';
				temp[j] = '\0';
			}
			i = 0;
			row++;
			count = row;
			col = 0;
		}
		else if(inputChar == KEY_UP) {
			if(count > 0) {
				count--;
				keyupHelper(input,beta,row,count);
				mvwprintw(input,input_y,input_x,"%s","dsh> ");
				wmove(input,input_y,input_x1);
				for(int j = 0; j < 100; j++) {
					alpha[j] = '\0';
					temp[j] = '\0';
				}
				for(int b = 0; b < strlen(beta[count]); b++) {
					alpha[b] = beta[count][b];
				}
				mvwprintw(input,input_y,input_x1,"%s",alpha);
				int x = 4 + i + 1;
				wmove(input,input_y,x);
			}
		}
		else if(inputChar == KEY_DOWN) {
			if(count != row-1) {
				count++;
				keydownHelper(input,beta,row,count);
				mvwprintw(input,input_y,input_x,"%s","dsh> ");
				wmove(input,input_y,input_x1);
				for(int j = 0; j < 100; j++) {
					alpha[j] = '\0';
					temp[j] = '\0';
				}
				for(int b = 0; b < strlen(beta[count]); b++) {
					alpha[b] = beta[count][b];
				}
				mvwprintw(input,input_y,input_x1,"%s",alpha);
				int x = 4 + i + 1;
				wmove(input,input_y,x);
			}
		}
		else {
			mvwprintw(input,input_y,input_x1,"%s",alpha);
			i++;
			col++;
		}
	}
	Nsession++;
	endwin();
	return 0;
}
