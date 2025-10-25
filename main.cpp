#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <malloc.h>
#include <math.h>
#include <windows.h>
#define field_size 8
#define depth_recure_bot_easy 4 // глубина рекурсии

// кнопки
#define bt_play_id 1
#define bt_quit_id 2
#define bt_easy 3
#define bt_hard 4
#define bt_back_C_to_M 5
#define bt_exit 6

HWND button_play;
HWND button_quit;
HWND button_easy;
HWND button_hard;
HWND button_back_C_to_M;
HWND button_exit;

// Размеры окон
int window_size_width = 1000;
int window_size_height = 1000;
int window_size_width2;
int window_size_height2;
#define square_size 50

char field[field_size][field_size]; // изменить

HFONT CreateCustomFont(int fontSize, const wchar_t* fontName) {
	return CreateFont(
		fontSize,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		fontName
	);
}

int dif_lvl;

int bot_field_eyes[field_size][field_size] = {
{90, -50, 1, 1, 1, 1, -50, 90},
{-50, -130, 1, 1, 1, 1, -130, -50},
{1, 1, 1, 1, 1, 1, 1, 1},
{1, 1, 1, 1, 1, 1, 1, 1},
{1, 1, 1, 1, 1, 1, 1, 1},
{1, 1, 1, 1, 1, 1, 1, 1},
{-50, -130, 1, 1, 1, 1, -130, -50},
{90, -50, 1, 1, 1, 1, -50, 90},
};

int wnd_opredelyator();

int turn_flag = 0;
int number_of_stones_on_field = 4;
int coord_flag = 0;

enum skip {
	not_skip_human,
	skip_human,
};
int skip_flag = not_skip_human;

enum resultation {
	playing,
	game_over,
};
int play_status = playing;

enum whos_turn {
	player_turn,
	bot_turn,
};
int whoturn = player_turn;

typedef struct coordination_cell {
	short i;
	short j;
	char i_char;
	whos_turn turn;
};
coordination_cell cell;

struct fields {
	char field[field_size][field_size];
	int num_stones;
	int turn;
	fields* prev;
	fields* next;
};

enum difficult_lvl {
	easy,
	hard,
};

fields* make_node(char field[field_size][field_size], int num_stones, int turn) {
	fields* ptr = (fields*)malloc(sizeof(fields));
	ptr->num_stones = num_stones;
	ptr->turn = turn;
	ptr->next = NULL;
	ptr->prev = NULL;

	int i = 0, j;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {
			ptr->field[i][j] = field[i][j];
			j++;
		}
		i++;
	}
	return ptr;
}

void interface_show_field(char field[field_size][field_size], char color) {
	int i = 0, j;
	printf("   0 1 2 3 4 5 6 7 \n");
	while (i < 8) {
		j = 0;
		printf("%d  ", i);
		while (j < 8) {
			if (field[i][j] == color)
				printf("* ");
			else if (field[i][j] == 'W')
				printf("@ ");
			else if (field[i][j] == 'B')
				printf("O ");
			else printf("_ ");
			j++;
		}
		printf("\n");
		i++;
	}
}

int optim_choice_lvl2(char field[field_size][field_size], char color) {
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else caps_color = 'W';

	int i = 0, j, position_sum = 0;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {

			if (field[i][j] == caps_color)
				position_sum += bot_field_eyes[i][j];

			j++;
		}
		i++;
	}
	return position_sum;
}

void show_field(char field[field_size][field_size]) {
	int i = 0, j;
	printf("   0 1 2 3 4 5 6 7 \n");
	while (i < 8) {
		j = 0;
		printf("%d  ", i);
		while (j < 8) {
			printf("%c ", field[i][j]);
			j++;
		}
		printf("\n");
		i++;
	}
}

// возвращает 1, когда нет ни одного камня одного цвета
// 0 - камни есть, игру продолжить можно
int full_domination_checking(char field[field_size][field_size]) {
	int i = 0, j, b_nums = 0, w_nums = 0;
	while (i < 8) {
		j = 0;
		while (j < 8) {
			if (field[i][j] == 'B')
				b_nums++;
			if (field[i][j] == 'W')
				w_nums++;
			j++;
		}
		i++;
	}
	if (w_nums != 0 && b_nums != 0)
		return 0;
	else return 1;
}

int checking_right(char field[field_size][field_size], int i, int j) {
	if (j == 7) return 0;
	int a = j + 1;
	char color = field[i][j];
	if (field[i][a] == color)
		return 0;
	do {
		if (field[i][a] == '_' || field[i][a] == 'b' || field[i][a] == 'w' || a == 7)
			return 0;
		a++;
	} while (field[i][a] != color);
	return 1;
}
int checking_left(char field[field_size][field_size], int i, int j) {
	if (j == 0) return 0;
	int a = j - 1;
	char color = field[i][j];
	if (field[i][a] == color)
		return 0;
	do {
		if (field[i][a] == '_' || field[i][a] == 'b' || field[i][a] == 'w' || a == 0)
			return 0;
		a--;
	} while (field[i][a] != color);
	return 1;
}
int checking_up(char field[field_size][field_size], int i, int j) {
	if (i == 0) return 0;
	int a = i - 1;
	char color = field[i][j];
	if (field[a][j] == color)
		return 0;
	do {
		if (field[a][j] == '_' || field[a][j] == 'b' || field[a][j] == 'w' || a == 0)
			return 0;
		a--;
	} while (field[a][j] != color);
	return 1;
}
int checking_down(char field[field_size][field_size], int i, int j) {
	if (i == 7) return 0;
	int a = i + 1;
	char color = field[i][j];
	if (field[a][j] == color)
		return 0;
	do {
		if (field[a][j] == '_' || field[a][j] == 'b' || field[a][j] == 'w' || a == 7)
			return 0;
		a++;
	} while (field[a][j] != color);
	return 1;
}
int checking_right_up(char field[field_size][field_size], int i, int j) {
	if (i == 0 || j == 7) return 0;
	int a = i - 1;
	int b = j + 1;
	char color = field[i][j];
	if (field[a][b] == color)
		return 0;
	do {
		if (field[a][b] == '_' || field[a][b] == 'b' || field[a][b] == 'w' || a == 0 || b == 7)
			return 0;
		a--;
		b++;
	} while (field[a][b] != color);
	return 1;
}
int checking_right_down(char field[field_size][field_size], int i, int j) {
	if (i == 7 || j == 7) return 0;
	int a = i + 1;
	int b = j + 1;
	char color = field[i][j];
	if (field[a][b] == color)
		return 0;
	do {
		if (field[a][b] == '_' || field[a][b] == 'b' || field[a][b] == 'w' || a == 7 || b == 7)
			return 0;
		a++;
		b++;
	} while (field[a][b] != color);
	return 1;
}
int checking_left_down(char field[field_size][field_size], int i, int j) {
	if (i == 7 || j == 0) return 0;
	int a = i + 1;
	int b = j - 1;
	char color = field[i][j];
	if (field[a][b] == color)
		return 0;
	do {
		if (field[a][b] == '_' || field[a][b] == 'b' || field[a][b] == 'w' || a == 7 || b == 0)
			return 0;
		a++;
		b--;
	} while (field[a][b] != color);
	return 1;
}
int checking_left_up(char field[field_size][field_size], int i, int j) {
	if (i == 0 || j == 0) return 0;
	int a = i - 1;
	int b = j - 1;
	char color = field[i][j];
	if (field[a][b] == color)
		return 0;
	do {
		if (field[a][b] == '_' || field[a][b] == 'b' || field[a][b] == 'w' || a == 0 || b == 0)
			return 0;
		a--;
		b--;
	} while (field[a][b] != color);
	return 1;
}

void reverse_right(char field[field_size][field_size], int i, int j) {
	int a = j + 1;
	char color = field[i][j];
	while (field[i][a] != color) {
		field[i][a] = color;
		a++;
	}
}
void reverse_left(char field[field_size][field_size], int i, int j) {
	int a = j - 1;
	char color = field[i][j];
	while (field[i][a] != color) {
		field[i][a] = color;
		a--;
	}
}
void reverse_up(char field[field_size][field_size], int i, int j) {
	int a = i - 1;
	char color = field[i][j];
	while (field[a][j] != color) {
		field[a][j] = color;
		a--;
	}
}
void reverse_down(char field[field_size][field_size], int i, int j) {
	int a = i + 1;
	char color = field[i][j];
	while (field[a][j] != color) {
		field[a][j] = color;
		a++;
	}
}
void reverse_right_up(char field[field_size][field_size], int i, int j) {
	int a = i - 1;
	int b = j + 1;
	char color = field[i][j];
	while (field[a][b] != color) {
		field[a][b] = color;
		a--;
		b++;
	}
}
void reverse_right_down(char field[field_size][field_size], int i, int j) {
	int a = i + 1;
	int b = j + 1;
	char color = field[i][j];
	while (field[a][b] != color) {
		field[a][b] = color;
		a++;
		b++;
	}
}
void reverse_left_up(char field[field_size][field_size], int i, int j) {
	int a = i - 1;
	int b = j - 1;
	char color = field[i][j];
	while (field[a][b] != color) {
		field[a][b] = color;
		a--;
		b--;
	}
}
void reverse_left_down(char field[field_size][field_size], int i, int j) {
	int a = i + 1;
	int b = j - 1;
	char color = field[i][j];
	while (field[a][b] != color) {
		field[a][b] = color;
		a++;
		b--;
	}
}
//Мы только что положили камень и теперь переворачиваем все остальные камни по правилам игры
//поэтому данная функция включает в себя проверки и перевороты
//НЕЛЬЗЯ ИСПОЛЬЗОВАТЬ ФУНКЦИЮ ПЕРЕВОРОТА БЕЗ ПРОВЕРКИ!
void reversing(char field[field_size][field_size], int i, int j) {
	if (checking_up(field, i, j)) reverse_up(field, i, j);
	if (checking_left(field, i, j)) reverse_left(field, i, j);
	if (checking_down(field, i, j)) reverse_down(field, i, j);
	if (checking_right(field, i, j)) reverse_right(field, i, j);

	if (checking_left_up(field, i, j)) reverse_left_up(field, i, j);
	if (checking_right_up(field, i, j)) reverse_right_up(field, i, j);
	if (checking_left_down(field, i, j)) reverse_left_down(field, i, j);
	if (checking_right_down(field, i, j)) reverse_right_down(field, i, j);
}

void clear_the_searchers(char field[field_size][field_size]) {
	int i = 0, j;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {
			if (field[i][j] == 'b' || field[i][j] == 'w')
				field[i][j] = '_';
			j++;
		}
		i++;
	}
}

void turn_searcher_left_down(char field[field_size][field_size], int a, int b, char color) {
	if (a == 7 || b == 0) //проверять дальше смысла нет
		return;

	// нам нужно знать цвет капсом
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else
		caps_color = 'W';

	int i = a + 1;
	int j = b - 1;
	if (field[i][j] == caps_color)
		return;

	do {
		if (field[i][j] == '_' || i == 7 || j == 0 || field[i][j] == 'b' || field[i][j] == 'w')
			return;
		i++;
		j--;
	} while (field[i][j] != caps_color);
	field[a][b] = color;
}
void turn_searcher_right_down(char field[field_size][field_size], int a, int b, char color) {
	if (a == 7 || b == 7) //проверять дальше смысла нет
		return;

	// нам нужно знать цвет капсом
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else
		caps_color = 'W';

	int i = a + 1;
	int j = b + 1;
	if (field[i][j] == caps_color)
		return;

	do {
		if (field[i][j] == '_' || i == 7 || j == 7 || field[i][j] == 'b' || field[i][j] == 'w')
			return;
		i++;
		j++;
	} while (field[i][j] != caps_color);
	field[a][b] = color;
}
void turn_searcher_right_up(char field[field_size][field_size], int a, int b, char color) {
	if (a == 0 || b == 7) //проверять дальше смысла нет
		return;

	// нам нужно знать цвет капсом
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else
		caps_color = 'W';

	int i = a - 1;
	int j = b + 1;
	if (field[i][j] == caps_color)
		return;

	do {
		if (field[i][j] == '_' || i == 0 || j == 7 || field[i][j] == 'b' || field[i][j] == 'w')
			return;
		i--;
		j++;
	} while (field[i][j] != caps_color);
	field[a][b] = color;
}
void turn_searcher_left_up(char field[field_size][field_size], int a, int b, char color) {
	if (a == 0 || b == 0) //проверять дальше смысла нет
		return;

	// нам нужно знать цвет капсом
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else
		caps_color = 'W';

	int i = a - 1;
	int j = b - 1;
	if (field[i][j] == caps_color)
		return;

	do {
		if (field[i][j] == '_' || i == 0 || j == 0 || field[i][j] == 'b' || field[i][j] == 'w')
			return;
		i--;
		j--;
	} while (field[i][j] != caps_color);
	field[a][b] = color;
}
void turn_searcher_left(char field[field_size][field_size], int a, int b, char color) {
	if (b == 0) //проверять дальше смысла нет
		return;

	// нам нужно знать цвет капсом
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else
		caps_color = 'W';

	//поиск по ряду комбинации color - another color * x - color
	int j = b - 1;
	if (field[a][j] == caps_color)
		return;

	do {
		if (field[a][j] == '_' || j == 0 || field[a][j] == 'b' || field[a][j] == 'w')
			return;
		j--;
	} while (field[a][j] != caps_color);
	field[a][b] = color;
}
void turn_searcher_right(char field[field_size][field_size], int a, int b, char color) {
	if (b == 7) //проверять дальше смысла нет
		return;

	// нам нужно знать цвет капсом
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else
		caps_color = 'W';

	//поиск по ряду комбинации color - another color * x - color
	int j = b + 1;
	if (field[a][j] == caps_color)
		return;

	do {
		if (field[a][j] == '_' || j == 7 || field[a][j] == 'b' || field[a][j] == 'w')
			return;
		j++;
	} while (field[a][j] != caps_color);
	field[a][b] = color;
}
void turn_searcher_down(char field[field_size][field_size], int a, int b, char color) {
	if (a == 7) //проверять дальше смысла нет
		return;

	// нам нужно знать цвет капсом
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else
		caps_color = 'W';

	//поиск по ряду комбинации color - another color * x - color
	int i = a + 1;
	if (field[i][b] == caps_color)
		return;

	do {
		if (field[i][b] == '_' || i == 7 || field[i][b] == 'b' || field[i][b] == 'w')
			return;
		i++;
	} while (field[i][b] != caps_color);
	field[a][b] = color;
}
void turn_searcher_up(char field[field_size][field_size], int a, int b, char color) {
	if (a == 0) //проверять дальше смысла нет
		return;

	// нам нужно знать цвет капсом
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else
		caps_color = 'W';

	//поиск по ряду комбинации color - another color * x - color
	int i = a - 1;
	if (field[i][b] == caps_color)
		return;

	do {
		if (field[i][b] == '_' || i == 0 || field[i][b] == 'b' || field[i][b] == 'w')
			return;
		i--;
	} while (field[i][b] != caps_color);
	field[a][b] = color;
}
//поиск возможных ходов для данного цвета. Их обозначение на поле
//b - черный, w - белый НЕ КАПСОМ!
// ПОЛЕ ДОЛЖНО БЫТЬ ПРЕДВОРИТЕЛЬНО ОЧИЩЕННО ОТ b И w
// функция возвращает 1, когда ход для данного цвета сделать можно. 0 - нельзя
int turn_searcher(char field[field_size][field_size], char color) {
	int i = 0, j;
	int flag = 0;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {

			// такая система из if-ов помогает ускорить вычисления
			if (field[i][j] == '_')
				turn_searcher_up(field, i, j, color);
			if (field[i][j] == '_')
				turn_searcher_down(field, i, j, color);
			if (field[i][j] == '_')
				turn_searcher_right(field, i, j, color);
			if (field[i][j] == '_')
				turn_searcher_left(field, i, j, color);
			if (field[i][j] == '_')
				turn_searcher_left_up(field, i, j, color);
			if (field[i][j] == '_')
				turn_searcher_right_up(field, i, j, color);
			if (field[i][j] == '_')
				turn_searcher_right_down(field, i, j, color);
			if (field[i][j] == '_')
				turn_searcher_left_down(field, i, j, color);

			if (field[i][j] == color)
				flag = 1;
			j++;
		}
		i++;
	}
	return flag;
}

int search_free_space(char field[field_size][field_size]) {
	int i = 0, j, num = 0;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {
			if (field[i][j] == '_' || field[i][j] == 'b' || field[i][j] == 'w')
				num++;
			j++;
		}
		i++;
	}
	return num;
}


void copy_field(char suda[field_size][field_size], char otsuda[field_size][field_size]) {
	int i = 0, j;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {
			suda[i][j] = otsuda[i][j];
			j++;
		}
		i++;
	}
}
int stones_of_this_color_counter(char field[field_size][field_size], char color) {
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else caps_color = 'W';

	int i = 0, j, num = 0;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {
			if (field[i][j] == caps_color)
				num++;
			j++;
		}
		i++;
	}
	return num;
}

void add_last(char field[field_size][field_size], char color) {
	char caps_color;
	if (color == 'b')
		caps_color = 'B';
	else caps_color = 'W';

	int i = 0, j;
	while (i, field_size) {
		j = 0;
		while (j < field_size) {
			if (field[i][j] != 'W' && field[i][j] != 'B') {
				field[i][j] = caps_color;
				reversing(field, i, j);
				return;
			}
			j++;
		}
		i++;
	}
}

void coordination_finder_hard_bot(char newfield[field_size][field_size], char oldfield[field_size][field_size], int* x, int* y) {
	int i = 0, j;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {
			if ((newfield[i][j] == 'W' && (oldfield[i][j] == '_' || oldfield[i][j] == 'b' || oldfield[i][j] == 'w')) ||
				(newfield[i][j] == 'B' && (oldfield[i][j] == '_' || oldfield[i][j] == 'b' || oldfield[i][j] == 'w'))) {
				*x = i;
				*y = j;
			}
			j++;
		}
		i++;
	}
}

// БОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТ

void bot_mind_hard_2(fields* prevptr, int turn, char saved_field[field_size][field_size], char now_color) {

	int depth = depth_recure_bot_easy, num_turns_thisfield;
	char another_color, caps_now_color;
	if (now_color == 'b') {
		another_color = 'w';
		caps_now_color = 'B';
	}
	else {
		another_color = 'b';
		caps_now_color = 'W';
	}

	int numis;
	if (turn % 2 == 0)
		numis = 0;
	else numis = 10000; //100

	int turns_exist_on_field = search_free_space(prevptr->field);
	if (turns_exist_on_field < depth)
		depth = turns_exist_on_field;
	if ((turns_exist_on_field == 1) && turn == 1) {
		copy_field(saved_field, prevptr->field);
		add_last(saved_field, now_color);
		free(prevptr);
		return;
	}

	turn_searcher(prevptr->field, now_color); //

	int i = 0, j;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {

			if (prevptr->field[i][j] == now_color) {
				// создаем новое поле
				// рассматриваем вариант при рек спуске
				char field[field_size][field_size];
				fields* ptr;
				copy_field(field, prevptr->field);
				clear_the_searchers(field);
				field[i][j] = caps_now_color;
				reversing(field, i, j);
				ptr = make_node(field, numis, turn);
				ptr->prev = prevptr;
				prevptr->next = ptr;

				num_turns_thisfield = turn_searcher(ptr->field, another_color); // ПРОВЕРКА НА НАЛИЧИЕ ХОДА
				clear_the_searchers(ptr->field);

				if (turn != depth && num_turns_thisfield != 0) { //рек спуск по полям

					bot_mind_hard_2(ptr, turn + 1, saved_field, another_color);

					// рек подъем
					// выбор оптимального хода
					if (turn % 2 == 0) { //четный
						if (prevptr->num_stones > ptr->num_stones)
							prevptr->num_stones = ptr->num_stones;
					}
					else { //нечетный
						if (prevptr->num_stones < ptr->num_stones) {
							prevptr->num_stones = ptr->num_stones;

							if (turn == 1) // если это оптимальный первый ход, то это то поле которое нам нужно
								copy_field(saved_field, ptr->field);
						}
						else if ((prevptr->num_stones == ptr->num_stones) && (turn == 1)) { // Рандом для хода с таким же результатом
							if ((rand() % 2) == 1)
								copy_field(saved_field, ptr->field);
						}
					}

				}
				else { // последнее рассматриваемое поле (остановка и следующий за ним рек подъем)
					int num_st;
					num_st = optim_choice_lvl2(ptr->field, now_color);
					ptr->num_stones = num_st;


					if (turn % 2 == 0) { //четный
						if (prevptr->num_stones > ptr->num_stones)
							prevptr->num_stones = ptr->num_stones;
					}
					else { //нечетный
						if (prevptr->num_stones < ptr->num_stones)
							prevptr->num_stones = ptr->num_stones;
						if (turn == 1) // если это оптимальный первый ход, то это то поле которое нам нужно
							copy_field(saved_field, ptr->field);
					}
				}
				free(ptr);
				prevptr->next = NULL;
			}
			j++;
		}
		i++;
	}
}

void bot_mind_hard(fields* prevptr, int turn, char saved_field[field_size][field_size], char now_color) {

	int depth = depth_recure_bot_easy, num_turns_thisfield;
	char another_color, caps_now_color;
	if (now_color == 'b') {
		another_color = 'w';
		caps_now_color = 'B';
	}
	else {
		another_color = 'b';
		caps_now_color = 'W';
	}

	int numis;
	if (turn % 2 == 0)
		numis = 0;
	else numis = 100;

	int turns_exist_on_field = search_free_space(prevptr->field);
	if (turns_exist_on_field < depth)
		depth = turns_exist_on_field;
	if ((turns_exist_on_field == 1) && turn == 1) {
		copy_field(saved_field, prevptr->field);
		add_last(saved_field, now_color);
		free(prevptr);
		return;
	}

	turn_searcher(prevptr->field, now_color);

	int i = 0, j;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {

			if (prevptr->field[i][j] == now_color) {
				// создаем новое поле
				// рассматриваем вариант при рек спуске
				char field[field_size][field_size];
				fields* ptr;
				copy_field(field, prevptr->field);
				clear_the_searchers(field);
				field[i][j] = caps_now_color;
				reversing(field, i, j);
				ptr = make_node(field, numis, turn);
				ptr->prev = prevptr;
				prevptr->next = ptr;

				num_turns_thisfield = turn_searcher(ptr->field, another_color); // ПРОВЕРКА НА НАЛИЧИЕ ХОДА
				clear_the_searchers(ptr->field);

				if (turn != depth && num_turns_thisfield != 0) { //рек спуск по полям

					bot_mind_hard(ptr, turn + 1, saved_field, another_color);

					// рек подъем
					// выбор оптимального хода
					if (turn % 2 == 0) { //четный
						if (prevptr->num_stones > ptr->num_stones)
							prevptr->num_stones = ptr->num_stones;
					}
					else { //нечетный
						if (prevptr->num_stones <= ptr->num_stones) { //
							prevptr->num_stones = ptr->num_stones;

							if (turn == 1) // если это оптимальный первый ход, то это то поле которое нам нужно ТУТ ПРОИСХОДИТ ОБЛОМ СКОРЕЕ ВСЕГО
								copy_field(saved_field, ptr->field); //облом из-за того что код не обрабатывает это место
						}
					}
				}
				else { // последнее рассматриваемое поле (остановка и следующий за ним рек подъем)
					int num_st;
					num_st = stones_of_this_color_counter(ptr->field, now_color);
					ptr->num_stones = num_st;


					if (turn % 2 == 0) { //четный
						if (prevptr->num_stones > ptr->num_stones)
							prevptr->num_stones = ptr->num_stones;
					}
					else { //нечетный
						if (prevptr->num_stones < ptr->num_stones) { //
							prevptr->num_stones = ptr->num_stones;
							if (turn == 1) // если это оптимальный первый ход, то это то поле которое нам нужно
								copy_field(saved_field, ptr->field);
						}
					}
				}
				free(ptr);
				prevptr->next = NULL;
			}
			j++;
		}
		i++;
	}
}

void bot_think_hard(char field[field_size][field_size], char bot_color, int* y, int* x) {
	char saved_field[field_size][field_size]; // ТУТ БУДЕТ ХОД БОТА
	char rezerv_field[field_size][field_size];
	copy_field(rezerv_field, field);
	// первое поле в структуре - это первоначальное поле, без совершенного хода
	// для него turn = 0
	fields* first_ptr = make_node(rezerv_field, 0, 0);
	first_ptr->num_stones = -5000;

	bot_mind_hard_2(first_ptr, 1, saved_field, bot_color);

	copy_field(field, saved_field);
	coordination_finder_hard_bot(field, rezerv_field, y, x);
}

void bot_think_easy(char field[field_size][field_size], char bot_color, int* y, int* x) {
	int counter;
	char rezerv_field[field_size][field_size];
	char caps_bot_color;
	if (bot_color == 'b')
		caps_bot_color = 'B';
	else caps_bot_color = 'W';

	int i = 0, j, stones_num, max_stones_num = 0, saved_i, saved_j;
	turn_searcher(field, bot_color);
	while (i < field_size) {
		j = 0;
		while (j < field_size) {

			if (field[i][j] == bot_color) {
				copy_field(rezerv_field, field);
				clear_the_searchers(rezerv_field);
				rezerv_field[i][j] = caps_bot_color;
				reversing(rezerv_field, i, j);
				stones_num = stones_of_this_color_counter(rezerv_field, bot_color);
				if (stones_num > max_stones_num) {
					max_stones_num = stones_num;
					saved_i = i;
					saved_j = j;
				}
				else if (stones_num == max_stones_num) { // Рандом для ходов с одинаковыми результатами
					if ((rand() % 2) == 1) {
						saved_i = i;
						saved_j = j;
					}
				}
			}
			j++;
		}
		i++;
	}

	clear_the_searchers(field);
	field[saved_i][saved_j] = caps_bot_color;
	reversing(field, saved_i, saved_j);
	*y = saved_i;
	*x = saved_j;
}

// БОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТБОТ

void cell_opredelyator() {
	switch (cell.i) {
	case 0:
		cell.i_char = 'A';
		break;
	case 1:
		cell.i_char = 'B';
		break;
	case 2:
		cell.i_char = 'C';
		break;
	case 3:
		cell.i_char = 'D';
		break;
	case 4:
		cell.i_char = 'E';
		break;
	case 5:
		cell.i_char = 'F';
		break;
	case 6:
		cell.i_char = 'G';
		break;
	case 7:
		cell.i_char = 'H';
		break;
	}
}

void field_draw(HDC dc, RECT clientRect, char field[field_size][field_size], HWND hwnd) {
	int boardWidth = square_size * field_size; // Размер поля в окне

	// поиск верхнего левого угла
	int startX = (clientRect.right - clientRect.left - boardWidth) / 2;
	int startY = (clientRect.bottom - clientRect.top - boardWidth) / 2;

	// рисование доски непосредственно: 1 этап СЛОТЫ ПРЯМОУГОЛЬНИКИ
	SelectObject(dc, GetStockObject(DC_BRUSH));
	SetDCBrushColor(dc, RGB(181, 155, 124)); // цвет доски

	// ГЕНЕРАЦИЯ СЛОТОВ ДОСКИ
	int i = 0, j;
	while (i < field_size) {
		j = 0;
		clear_the_searchers(field);
		turn_searcher(field, 'b');
		while (j < field_size) {
			Rectangle(dc, startX + square_size * i, startY + square_size * j, startX + square_size * (i + 1), startY + square_size * (j + 1));
			if (field[j][i] == 'B' || field[j][i] == 'W' || field[j][i] == 'b') {
				SelectObject(dc, GetStockObject(DC_BRUSH));
				if (field[j][i] == 'B') {
					SetDCBrushColor(dc, RGB(0, 0, 0));
					Ellipse(dc, startX + i * square_size + 3, startY + j * square_size + 3, startX + (i + 1) * square_size - 3, startY + (j + 1) * square_size - 3);
				}
				else if (field[j][i] == 'W') {
					SetDCBrushColor(dc, RGB(255, 255, 255));
					Ellipse(dc, startX + i * square_size + 3, startY + j * square_size + 3, startX + (i + 1) * square_size - 3, startY + (j + 1) * square_size - 3);
				}
				else if (field[j][i] == 'b' && whoturn == player_turn) {
					SetDCBrushColor(dc, RGB(120, 120, 255));
					Ellipse(dc, startX + i * square_size + (square_size / 4) + 2, startY + j * square_size + (square_size / 4) + 2, startX + (i + 1) * square_size - (square_size / 4) - 2, startY + (j + 1) * square_size - (square_size / 4) - 2);
				}

				SetDCBrushColor(dc, RGB(181, 155, 124)); // вернуть цвет который был
			}
			j++;
		}
		i++;
	}

	HFONT hFont = CreateCustomFont(24, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(dc, hFont);

	SetTextColor(dc, RGB(0, 0, 0)); // Black text color
	SetBkMode(dc, TRANSPARENT); // Transparent background for text

	wchar_t labelText[2];
	SIZE textSize;

	// Draw letters on the left
	for (i = 0; i < field_size; i++) {
		swprintf(labelText, 2, L"%c", L'A' + i);
		TextOutW(dc, startX - 20, startY + i * square_size + square_size / 2 - 10, labelText, wcslen(labelText));
	}

	// Draw numbers on the top
	int horizontalOffset = -5; // Adjust this value to move the numbers more to the left
	for (i = 0; i < field_size; i++) {
		swprintf(labelText, 2, L"%d", i + 1);
		GetTextExtentPoint32W(dc, labelText, wcslen(labelText), &textSize);
		TextOutW(dc, startX + i * square_size + (square_size + textSize.cx) / 2 + horizontalOffset, startY - textSize.cy - 5, labelText, wcslen(labelText));
	}

	SelectObject(dc, hOldFont);
	DeleteObject(hFont);

	// Draw framed score display at the top
	int black_score = stones_of_this_color_counter(field, 'b');
	int white_score = stones_of_this_color_counter(field, 'w');
	wchar_t scoreText[50];

	hFont = CreateCustomFont(24, L"Arial");
	hOldFont = (HFONT)SelectObject(dc, hFont);

	SetTextColor(dc, RGB(0, 0, 0)); // Black text color
	SetBkMode(dc, TRANSPARENT); // Transparent background for text

	int scoreBoxWidth = 200;
	int scoreBoxHeight = 40;
	int scoreBoxX = (clientRect.right - clientRect.left - scoreBoxWidth) / 2;
	int scoreBoxY = startY - scoreBoxHeight - 40;

	// Draw score frame
	Rectangle(dc, scoreBoxX, scoreBoxY, scoreBoxX + scoreBoxWidth, scoreBoxY + scoreBoxHeight);

	// Draw black score
	swprintf(scoreText, 50, L"%d", black_score);
	TextOutW(dc, scoreBoxX + 10, scoreBoxY + 10, scoreText, wcslen(scoreText));
	SetDCBrushColor(dc, RGB(0, 0, 0));
	Ellipse(dc, scoreBoxX + 35, scoreBoxY + 10, scoreBoxX + 55, scoreBoxY + 30);
	// Draw white score
	swprintf(scoreText, 50, L"%d", white_score);
	TextOutW(dc, scoreBoxX + 150, scoreBoxY + 10, scoreText, wcslen(scoreText));
	SetDCBrushColor(dc, RGB(255, 255, 255));
	Ellipse(dc, scoreBoxX + 175, scoreBoxY + 10, scoreBoxX + 195, scoreBoxY + 30);

	if (coord_flag != 0) {
		wchar_t coordText[50];
		cell_opredelyator();
		if (cell.turn == bot_turn)
			swprintf(coordText, 50, L"ХОД БОТА: %c %d", cell.i_char, cell.j + 1);
		else swprintf(coordText, 50, L"ХОД ИГРОКА: %c %d", cell.i_char, cell.j + 1);
		TextOutW(dc, (window_size_width / 2) - 75, (window_size_height / 2) + 235, coordText, wcslen(coordText));
	}

	if (play_status == game_over) {
		wchar_t resultText[50];
		if (stones_of_this_color_counter(field, 'w') > stones_of_this_color_counter(field, 'b')) { // бот победил
			SetTextColor(dc, RGB(255, 0, 0));
			swprintf(resultText, 50, L"ВЫ ПРОИГРАЛИ");
			TextOutW(dc, (window_size_width / 2) - 75, (window_size_height / 10), resultText, wcslen(resultText));
		}
		else if (stones_of_this_color_counter(field, 'w') < stones_of_this_color_counter(field, 'b')) { // игрок победил
			SetTextColor(dc, RGB(0, 255, 0));
			swprintf(resultText, 50, L"ВЫ ВЫИГРАЛИ");
			TextOutW(dc, (window_size_width / 2) - 75, (window_size_height / 10), resultText, wcslen(resultText));
		}
		else { // ничья
			SetTextColor(dc, RGB(200, 200, 200));
			swprintf(resultText, 50, L"НИЧЬЯ");
			TextOutW(dc, (window_size_width / 2) - 60, (window_size_height / 10), resultText, wcslen(resultText));
		}

	}

	SelectObject(dc, hOldFont);
	DeleteObject(hFont);
}

LRESULT CALLBACK play_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		RECT clientRect;
		HDC dc = BeginPaint(hwnd, &ps);
		window_size_width = window_size_width2;
		window_size_height = window_size_height2;
		GetClientRect(hwnd, &clientRect);
		field_draw(dc, clientRect, field, hwnd);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_SIZE: {
		RECT coordinate;
		GetWindowRect(hwnd, &coordinate);
		window_size_width = LOWORD(lparam);
		window_size_height = HIWORD(lparam);
		window_size_width2 = window_size_width;
		window_size_height2 = window_size_height;
		MoveWindow(button_exit, (window_size_width / 4), (window_size_height / 2) + 290, (window_size_width / 2), (window_size_height / 20), true);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	}
	case WM_LBUTTONDOWN: {
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		int boardWidth = square_size * field_size; // Размер поля в окне

		// поиск верхнего левого угла
		int startX = (clientRect.right - clientRect.left - boardWidth) / 2;
		int startY = (clientRect.bottom - clientRect.top - boardWidth) / 2;

		int xPos = LOWORD(lparam); // ПОЗИЦИЯ МЫШКИ
		int yPos = HIWORD(lparam);

		if (((xPos >= startX) && (xPos < startX + (square_size * field_size)) && // если курсор внутри поля
			(yPos >= startY) && (yPos < startY + (square_size * field_size))) ||
			(skip_flag == skip_human)) {

			if (skip_flag != skip_human) {

				int x_coordinate_field = (xPos - startX) / square_size; //координаты относительно поля
				int y_coordinate_field = (yPos - startY) / square_size;
				printf("%d %d\n", y_coordinate_field, x_coordinate_field);
				// ДЛЯ СТРОКИ КООРДИНАТ
				cell.i = y_coordinate_field;
				cell.j = x_coordinate_field;
				cell.turn = player_turn;
				coord_flag = 1;

				//Ход игрока
				if (field[y_coordinate_field][x_coordinate_field] == 'b') {
					field[y_coordinate_field][x_coordinate_field] = 'B';
					whoturn = bot_turn;
					reversing(field, y_coordinate_field, x_coordinate_field);
					number_of_stones_on_field++;
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
					turn_flag = 0;
					clear_the_searchers(field);
					show_field(field);
				}
				else {
					printf("Так ставить фишку нельзя\n");
					break;
				}
			}
			else skip_flag = not_skip_human;

			Sleep(200);
			// подсчеты
			int is_turn;
			int number_of_stones_on_field = stones_of_this_color_counter(field, 'b') + stones_of_this_color_counter(field, 'w');
			if (number_of_stones_on_field != 64 && turn_flag != 2 && (full_domination_checking(field) == 0) && (turn_searcher(field, 'w') == 1)) { // ход бота
				int x_coordinate_field;
				int y_coordinate_field;
				if (dif_lvl == easy)
					bot_think_easy(field, 'w', &y_coordinate_field, &x_coordinate_field);
				else if (dif_lvl == hard)
					bot_think_hard(field, 'w', &y_coordinate_field, &x_coordinate_field);
				whoturn = player_turn;
				// ДЛЯ КООРДИНАТ
				printf("BOT: %d %d\n", y_coordinate_field, x_coordinate_field);
				cell.i = y_coordinate_field;
				cell.j = x_coordinate_field;
				cell.turn = bot_turn;
				coord_flag = 1;
				clear_the_searchers(field);
				number_of_stones_on_field++;
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
				turn_flag = 0;
				show_field(field);
			}
			else if (full_domination_checking(field) == 1) { // бот проиграл
				printf("Бот проиграл\n");
				play_status = game_over;
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
				break;
			}
			else if (number_of_stones_on_field == 64 || turn_flag == 2) { //игра завершилась
				play_status = game_over;
				printf("Game over\n");
				turn_flag = 0;
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
				break;
			}
			else if (turn_searcher(field, 'w') == 0) { // хода у бота нет
				turn_flag++;
				printf("Бот не может сделать ход. Ход передается игроку flag %d\n", turn_flag);
				whoturn = player_turn;
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
			}

			if (number_of_stones_on_field == 64 || turn_flag == 2 || (full_domination_checking(field) == 1) || (turn_searcher(field, 'b') == 0)) { // ход игрока (если он не может сделать)

				if (full_domination_checking(field) == 1) { // игрок проиграл
					play_status = game_over;
					printf("Человек проиграл\n");
					play_status = game_over;
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
					break;
				}
				else if (number_of_stones_on_field == 64 || turn_flag == 2) { //игра завершилась
					play_status = game_over;
					printf("Game over2\n");
					turn_flag = 0;
					play_status = game_over;
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
					break;
				}
				else if (turn_searcher(field, 'b') == 0) { // хода у игрока нет
					turn_flag++;
					if (turn_flag == 1) {
						printf("Игрок не может сделать ход. Ход передается боту\n");
						// ХОД БОТА
						skip_flag = skip_human;
						SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));

					}
					else if (turn_flag == 2) { // игра завершилась
						printf("game over3 никто не может сделать ход\n");
						play_status = game_over;
						turn_flag = 0;
						InvalidateRect(hwnd, NULL, TRUE);
						UpdateWindow(hwnd);
						break;
					}
				}
			}
		}
		break;
	}
	case WM_COMMAND: {
		if (LOWORD(wparam) == bt_exit) {
			DestroyWindow(hwnd);
			wnd_opredelyator();
		}
	}
	default:
		return DefWindowProcA(hwnd, message, wparam, lparam);
	}
}

void play_window(difficult_lvl lvl) {
	//window_size_width = 1000;
	//window_size_height = 1000;
	WNDCLASSA wcl3;
	memset(&wcl3, 0, sizeof(WNDCLASSA));
	wcl3.lpszClassName = "playWin";
	wcl3.lpfnWndProc = play_WndProc;
	wcl3.hbrBackground = CreateSolidBrush(RGB(0, 128, 128)); // background
	RegisterClassA(&wcl3);
	HWND hwnd_choise_window = CreateWindowA("playWin", "REVERSI", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window_size_width, window_size_height, NULL, NULL, NULL, NULL);
	MSG msg;

	button_exit = CreateWindowA("Button", "Выход в главное меню", WS_VISIBLE | WS_CHILD, (window_size_width / 4), (window_size_height / 4), (window_size_width / 2), (window_size_height / 10), hwnd_choise_window, (HMENU)bt_exit, NULL, NULL);

	ShowWindow(hwnd_choise_window, SW_SHOWNORMAL);
	while (GetMessage(&msg, NULL, 0, 0)) {
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK choice_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	case WM_COMMAND: {
		if (LOWORD(wparam) == bt_easy) {
			DestroyWindow(hwnd);
			dif_lvl = easy;
			play_window(easy);
		}
		else if (LOWORD(wparam) == bt_hard) {
			DestroyWindow(hwnd);
			dif_lvl = hard;
			play_window(hard);
		}
		else if (LOWORD(wparam) == bt_back_C_to_M) {
			DestroyWindow(hwnd);
			wnd_opredelyator();
		}
	}
	case WM_SIZE: {
		RECT coordinate;
		GetWindowRect(hwnd, &coordinate);
		window_size_width = LOWORD(lparam);
		window_size_height = HIWORD(lparam);
		MoveWindow(button_easy, (window_size_width / 4), (window_size_height / 4), (window_size_width / 2), (window_size_height / 10), true);
		MoveWindow(button_hard, (window_size_width / 4), (window_size_height / 3) + (window_size_height / 7), (window_size_width / 2), (window_size_height / 10), true);
		MoveWindow(button_back_C_to_M, (window_size_width / 4), (window_size_height / 2) + (window_size_height / 5), (window_size_width / 2), (window_size_height / 10), true);
		break;
	}
	default:
		return DefWindowProcA(hwnd, message, wparam, lparam);
	}
}

void difficult_choice_window() {
	window_size_width = 1000; //750
	window_size_height = 1000;
	WNDCLASSA wcl2;
	memset(&wcl2, 0, sizeof(WNDCLASSA));
	wcl2.lpszClassName = "choiceWin";
	wcl2.lpfnWndProc = choice_WndProc;
	wcl2.hbrBackground = CreateSolidBrush(RGB(0, 128, 128)); // background
	RegisterClassA(&wcl2);
	HWND hwnd_choise_window = CreateWindowA("choiceWin", "REVERSI", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window_size_width, window_size_height, NULL, NULL, NULL, NULL);
	MSG msg;

	button_easy = CreateWindowA("Button", "Легкий", WS_VISIBLE | WS_CHILD, (window_size_width / 4), (window_size_height / 4), (window_size_width / 2), (window_size_height / 10), hwnd_choise_window, (HMENU)bt_easy, NULL, NULL);
	button_hard = CreateWindowA("Button", "Сложный", WS_VISIBLE | WS_CHILD, (window_size_width / 4), (window_size_height / 3) + (window_size_height / 7), (window_size_width / 2), (window_size_height / 10), hwnd_choise_window, (HMENU)bt_hard, NULL, NULL);
	button_back_C_to_M = CreateWindowA("Button", "Назад", WS_VISIBLE | WS_CHILD, (window_size_width / 4), (window_size_height / 2) + (window_size_height / 5), (window_size_width / 2), (window_size_height / 10), hwnd_choise_window, (HMENU)bt_back_C_to_M, NULL, NULL);

	ShowWindow(hwnd_choise_window, SW_SHOWNORMAL);

	while (GetMessage(&msg, NULL, 0, 0)) {
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK begining_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		RECT clientRect;
		HDC dc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &clientRect);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_COMMAND: {
		if (LOWORD(wparam) == bt_quit_id) {
			DestroyWindow(hwnd);
			PostQuitMessage(0);
		}
		else if (LOWORD(wparam) == bt_play_id) {
			DestroyWindow(hwnd);
			difficult_choice_window();
		}
		break;
	}
	case WM_SIZE: { //меняет растяжение кнопок в случае растяжении окна
		RECT coordinate;
		GetWindowRect(hwnd, &coordinate);
		window_size_width = LOWORD(lparam);
		window_size_height = HIWORD(lparam);
		MoveWindow(button_play, (window_size_width / 4), (window_size_height / 4), (window_size_width / 2), (window_size_height / 10), true);
		MoveWindow(button_quit, (window_size_width / 4), (window_size_height / 2) + (window_size_height / 5), (window_size_width / 2), (window_size_height / 10), true);
		break;
	}
	default:
		return DefWindowProcA(hwnd, message, wparam, lparam);
	}
}

int wnd_opredelyator() {
	// создание поля
	int x = 0, y;
	while (x < field_size) {
		y = 0;
		while (y < field_size) {
			field[x][y] = '_';
			y++;
		}
		x++;
	}
	field[3][3] = 'W';
	field[3][4] = 'B';
	field[4][4] = 'W';
	field[4][3] = 'B';
	whoturn = player_turn;
	play_status = playing;
	coord_flag = 0;
	window_size_width = 1000; //750
	window_size_height = 1000;
	WNDCLASSA wcl;
	memset(&wcl, 0, sizeof(WNDCLASSA));
	wcl.lpszClassName = "MenuWindow";
	wcl.lpfnWndProc = begining_WndProc;
	wcl.hbrBackground = CreateSolidBrush(RGB(0, 128, 128)); // ФОН

	RegisterClassA(&wcl);
	HWND hwnd_main_menu = CreateWindowA("MenuWindow", "REVERSI", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window_size_width, window_size_height, NULL, NULL, NULL, NULL);
	button_play = CreateWindowA("Button", "Играть!", WS_VISIBLE | WS_CHILD, (window_size_width / 4), (window_size_height / 4), (window_size_width / 2), (window_size_height / 10), hwnd_main_menu, (HMENU)bt_play_id, NULL, NULL);
	button_quit = CreateWindowA("Button", "Выход", WS_VISIBLE | WS_CHILD, (window_size_width / 4), (window_size_height / 2) + (window_size_height / 5), (window_size_width / 2), (window_size_height / 10), hwnd_main_menu, (HMENU)bt_quit_id, NULL, NULL);

	MSG msg;
	ShowWindow(hwnd_main_menu, SW_SHOWNORMAL);

	while (GetMessage(&msg, NULL, 0, 0)) {
		DispatchMessage(&msg);
	}
	return 0;
}

void winning_return(char field[field_size][field_size], int* black_win, int* black_lose, int* drawn_game) {
	int black, white;
	black = stones_of_this_color_counter(field, 'b');
	white = stones_of_this_color_counter(field, 'w');
	if (black > white) {
		printf("ЧЕРНЫЕ ПОБЕДИЛИ: %d черных, %d белых\n", black, white);
		(*black_win)++;
	}
	else if (black < white) {
		printf("БЕЛЫЕ ПОБЕДИЛИ: %d черных, %d белых\n", black, white);
		(*black_lose)++;
	}
	else if (black == white) {
		printf("НИЧЬЯ: %d черных, %d белых\n", black, white);
		(*drawn_game)++;
	}
}

void bot_vs_bot(difficult_lvl black, difficult_lvl white, int* black_win, int* black_lose, int* drawn_game) {
	int xx, yy;

	int* x = &xx;
	int* y = &yy;

	char field[field_size][field_size];
	int i = 0, j;
	while (i < field_size) {
		j = 0;
		while (j < field_size) {
			field[i][j] = '_';
			j++;
		}
		i++;
	}
	field[3][3] = 'W';
	field[3][4] = 'B';
	field[4][3] = 'B';
	field[4][4] = 'W';

	char bot_color_1 = 'b';
	char bot_color_2 = 'w';
	int a, b, flag;
	int number_of_stones_on_field = 4; //счетчик начинается отсюда
	do {
		flag = 0;

		//FIRST BOT (BLACK)
		turn_searcher(field, bot_color_1);
		if (full_domination_checking(field)) {
			printf("ПЕРВЫЙ БОТ (ЧЕРНЫЙ) ПРОИГРАЛ! У НЕГО НЕ ОСТАЛОСЬ КАМНЕЙ!\n");
			(*black_lose)++;
			return;
		}

		if (turn_searcher(field, bot_color_1)) {
			clear_the_searchers(field);
			if (black == easy)
				bot_think_easy(field, bot_color_1, y, x);
			else bot_think_hard(field, bot_color_1, y, x);
			number_of_stones_on_field++;
		}
		else {
			//printf("ПЕРВЫЙ БОТ НЕ МОЖЕТ СДЕЛАТЬ ХОД! ХОД ПЕРЕДАЕТСЯ ВТОРОМУ БОТУ\n");
			flag++;
		}

		//SECOND BOT (WHITE)
		turn_searcher(field, bot_color_2);
		if (full_domination_checking(field)) {
			printf("ВТОРОЙ БОТ (БЕЛЫЙ) ПРОИГРАЛ! У НЕГО НЕ ОСТАЛОСЬ КАМНЕЙ!\n");
			(*black_win)++;
			return;
		}
		if (turn_searcher(field, bot_color_2)) {
			clear_the_searchers(field);
			if (white == easy)
				bot_think_easy(field, bot_color_2, y, x);
			else bot_think_hard(field, bot_color_2, y, x);
			number_of_stones_on_field++;
		}
		else {
			flag++;
		}
	} while (number_of_stones_on_field != 64 && flag != 2);
	winning_return(field, black_win, black_lose, drawn_game);
}

int main() {
	setlocale(LC_ALL, "RUS");
	srand(time(NULL));

	wnd_opredelyator();
	//init_test();

	/*int iteration = 0;
	int black_win = 0, black_lose = 0, drawn_game = 0;
	unsigned __int64 start, end, summa_time = 0;*/
	//printf("----------------EASY_VS_EASY----------------\n");
	//while (iteration < 100) {
	//	bot_vs_bot(easy, easy, &black_win, &black_lose, &drawn_game);
	//	iteration++;
	//}
	//printf("\nИТОГО:\n%d%% черные победили\n%d%% белые победили\n%d%% ничья\n\n", black_win, black_lose, drawn_game);

	/*iteration = 0;
	black_win = 0;
	black_lose = 0;
	drawn_game = 0;
	printf("----------------HARD_VS_HARD----------------\n");
	while (iteration < 10) {
		start = __rdtsc();
		bot_vs_bot(hard, hard, &black_win, &black_lose, &drawn_game);
		end = __rdtsc();
		if (iteration > 4)
			summa_time += end - start;
		iteration++;
	}
	printf("\nИТОГО:\n%d%% черные победили\n%d%% белые победили\n%d%% ничья\n\n", black_win, black_lose, drawn_game);
	printf("ВРЕМЯ: %I64d\n", summa_time / 5);*/
	//iteration = 0;
	//black_win = 0;
	//black_lose = 0;
	//drawn_game = 0;
	//printf("----------------HARD_VS_EASY----------------\n");
	//while (iteration < 100) {
	//	bot_vs_bot(hard, easy, &black_win, &black_lose, &drawn_game);
	//	iteration++;
	//}
	//printf("\nИТОГО:\n%d%% черные победили\n%d%% белые победили\n%d%% ничья\n\n", black_win, black_lose, drawn_game);

	//iteration = 0;
	//black_win = 0;
	//black_lose = 0;
	//drawn_game = 0;
	//printf("----------------EASY_VS_HARD----------------\n");
	//while (iteration < 100) {
	//	bot_vs_bot(easy, hard, &black_win, &black_lose, &drawn_game);
	//	iteration++;
	//}
	//printf("\nИТОГО:\n%d%% черные победили\n%d%% белые победили\n%d%% ничья\n\n", black_win, black_lose, drawn_game);
	//return 0;
}