#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <string>

#include "io.h"
#include "pair.h"
#include "character.h"
#include "poke327.h"
// #include "pokemon.h"

#define TRAINER_LIST_FIELD_WIDTH 46

#define FLEE_CHANCE  50

typedef struct io_message {
  /* Will print " --more-- " at end of line when another message follows. *
   * Leave 10 extra spaces for that.                                      */
  char msg[71];
  struct io_message *next;
} io_message_t;

static io_message_t *io_head, *io_tail;

void io_init_terminal(void)
{
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void io_reset_terminal(void)
{
  endwin();

  while (io_head) {
    io_tail = io_head;
    io_head = io_head->next;
    free(io_tail);
  }
  io_tail = NULL;
}

void io_queue_message(const char *format, ...)
{
  io_message_t *tmp;
  va_list ap;

  if (!(tmp = (io_message_t *) malloc(sizeof (*tmp)))) {
    perror("malloc");
    exit(1);
  }

  tmp->next = NULL;

  va_start(ap, format);

  vsnprintf(tmp->msg, sizeof (tmp->msg), format, ap);

  va_end(ap);

  if (!io_head) {
    io_head = io_tail = tmp;
  } else {
    io_tail->next = tmp;
    io_tail = tmp;
  }
}

static void io_print_message_queue(uint32_t y, uint32_t x)
{
  while (io_head) {
    io_tail = io_head;
    attron(COLOR_PAIR(COLOR_CYAN));
    mvprintw(y, x, "%-80s", io_head->msg);
    attroff(COLOR_PAIR(COLOR_CYAN));
    io_head = io_head->next;
    if (io_head) {
      attron(COLOR_PAIR(COLOR_CYAN));
      mvprintw(y, x + 70, "%10s", " --more-- ");
      attroff(COLOR_PAIR(COLOR_CYAN));
      refresh();
      getch();
    }
    free(io_tail);
  }
  io_tail = NULL;
}

/* === Subwindow Management === */
WINDOW *open_popup(int height, int width, int y, int x) {
  WINDOW *popup;

  popup = newwin(height, width, y, x);
  keypad(popup, TRUE);
  scrollok(popup, TRUE);
  idlok(popup, TRUE);
  wsetscrreg(popup, 5, 16);
  box(popup, 0, 0);
  wrefresh(popup);

  return popup;
};
void close_popup(WINDOW *popup) {
  wborder(popup, ' ',' ',' ',' ',' ',' ',' ',' ');
  wclear(popup);
  wrefresh(popup);
  delwin(popup);
};


/**************************************************************************
 * Compares trainer distances from the PC according to the rival distance *
 * map.  This gives the approximate distance that the PC must travel to   *
 * get to the trainer (doesn't account for crossing buildings).  This is  *
 * not the distance from the NPC to the PC unless the NPC is a rival.     *
 *                                                                        *
 * Not a bug.                                                             *
 **************************************************************************/
static int compare_trainer_distance(const void *v1, const void *v2)
{
  const character *const *c1 = (const character * const *) v1;
  const character *const *c2 = (const character * const *) v2;

  return (world.rival_dist[(*c1)->pos[dim_y]][(*c1)->pos[dim_x]] -
          world.rival_dist[(*c2)->pos[dim_y]][(*c2)->pos[dim_x]]);
}

static character *io_nearest_visible_trainer()
{
  character **c, *n;
  uint32_t x, y, count;

  c = (character **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = world.cur_map->cmap[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  n = c[0];

  free(c);

  return n;
}

void io_display()
{
  uint32_t y, x;
  character *c;

  clear();
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (world.cur_map->cmap[y][x]) {
        mvaddch(y + 1, x, world.cur_map->cmap[y][x]->symbol);
      } else {
        switch (world.cur_map->map[y][x]) {
        case ter_boulder:
          attron(COLOR_PAIR(COLOR_MAGENTA));
          mvaddch(y + 1, x, BOULDER_SYMBOL);
          attroff(COLOR_PAIR(COLOR_MAGENTA));
          break;
        case ter_mountain:
          attron(COLOR_PAIR(COLOR_MAGENTA));
          mvaddch(y + 1, x, MOUNTAIN_SYMBOL);
          attroff(COLOR_PAIR(COLOR_MAGENTA));
          break;
        case ter_tree:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, TREE_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_forest:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, FOREST_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_path:
        case ter_bailey:
          attron(COLOR_PAIR(COLOR_YELLOW));
          mvaddch(y + 1, x, PATH_SYMBOL);
          attroff(COLOR_PAIR(COLOR_YELLOW));
          break;
        case ter_gate:
          attron(COLOR_PAIR(COLOR_YELLOW));
          mvaddch(y + 1, x, GATE_SYMBOL);
          attroff(COLOR_PAIR(COLOR_YELLOW));
          break;
        case ter_mart:
          attron(COLOR_PAIR(COLOR_BLUE));
          mvaddch(y + 1, x, POKEMART_SYMBOL);
          attroff(COLOR_PAIR(COLOR_BLUE));
          break;
        case ter_center:
          attron(COLOR_PAIR(COLOR_RED));
          mvaddch(y + 1, x, POKEMON_CENTER_SYMBOL);
          attroff(COLOR_PAIR(COLOR_RED));
          break;
        case ter_grass:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, TALL_GRASS_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_clearing:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, SHORT_GRASS_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_water:
          attron(COLOR_PAIR(COLOR_CYAN));
          mvaddch(y + 1, x, WATER_SYMBOL);
          attroff(COLOR_PAIR(COLOR_CYAN));
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          attron(COLOR_PAIR(COLOR_CYAN));
          mvaddch(y + 1, x, ERROR_SYMBOL);
          attroff(COLOR_PAIR(COLOR_CYAN)); 
       }
      }
    }
  }

  mvprintw(23, 1, "PC position is (%2d,%2d) on map %d%cx%d%c.",
           world.pc.pos[dim_x],
           world.pc.pos[dim_y],
           abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_x] - (WORLD_SIZE / 2) >= 0 ? 'E' : 'W',
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2) <= 0 ? 'N' : 'S');
  mvprintw(22, 1, "%d known %s.", world.cur_map->num_trainers,
           world.cur_map->num_trainers > 1 ? "trainers" : "trainer");
  mvprintw(22, 30, "Nearest visible trainer: ");
  if ((c = io_nearest_visible_trainer())) {
    attron(COLOR_PAIR(COLOR_RED));
    mvprintw(22, 55, "%c at vector %d%cx%d%c.",
             c->symbol,
             abs(c->pos[dim_y] - world.pc.pos[dim_y]),
             ((c->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              'N' : 'S'),
             abs(c->pos[dim_x] - world.pc.pos[dim_x]),
             ((c->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              'W' : 'E'));
    attroff(COLOR_PAIR(COLOR_RED));
  } else {
    attron(COLOR_PAIR(COLOR_BLUE));
    mvprintw(22, 55, "NONE.");
    attroff(COLOR_PAIR(COLOR_BLUE));
  }

  // std::string buddies_hud = "Buddies: [" + std::to_string(world.pc.num_buddies) 
  //               + "/6] | @: " + world.pc.buddy[0]->get_species() 
	// 							+ " L:" + std::to_string(world.pc.buddy[0]->get_lvl())
	// 							+ "(" + std::to_string(world.pc.buddy[0]->get_chp()) 
	// 										+ "/" + std::to_string(world.pc.buddy[0]->get_hp()) + ")";
  // mvprintw(23, 40, "%40s", buddies_hud.c_str());

  io_print_message_queue(0, 0);

  refresh();
}

uint32_t io_teleport_pc(pair_t dest)
{
  /* Just for fun. And debugging.  Mostly debugging. */

  do {
    dest[dim_x] = rand_range(1, MAP_X - 2);
    dest[dim_y] = rand_range(1, MAP_Y - 2);
  } while (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]                  ||
           move_cost[char_pc][world.cur_map->map[dest[dim_y]]
                                                [dest[dim_x]]] ==
             DIJKSTRA_PATH_MAX                                            ||
           world.rival_dist[dest[dim_y]][dest[dim_x]] < 0);

  return 0;
}

static void io_scroll_trainer_list(char (*s)[TRAINER_LIST_FIELD_WIDTH],
                                   uint32_t count)
{
  uint32_t offset;
  uint32_t i;

  offset = 0;

  while (1) {
    for (i = 0; i < 13; i++) {
      mvprintw(i + 6, 19, " %-40s ", s[i + offset]);
    }
    switch (getch()) {
    case KEY_UP:
      if (offset) {
        offset--;
      }
      break;
    case KEY_DOWN:
      if (offset < (count - 13)) {
        offset++;
      }
      break;
    case 27:
      return;
    }

  }
}

static void io_list_trainers_display(npc **c, uint32_t count)
{
  uint32_t i;
  char (*s)[TRAINER_LIST_FIELD_WIDTH]; /* pointer to array of 40 char */

  s = (char (*)[TRAINER_LIST_FIELD_WIDTH]) malloc(count * sizeof (*s));

  mvprintw(3, 19, " %-40s ", "");
  /* Borrow the first element of our array for this string: */
  snprintf(s[0], TRAINER_LIST_FIELD_WIDTH, "You know of %d trainers:", count);
  mvprintw(4, 19, " %-40s ", *s);
  mvprintw(5, 19, " %-40s ", "");

  for (i = 0; i < count; i++) {
    snprintf(s[i], TRAINER_LIST_FIELD_WIDTH, "%16s %c: %2d %s by %2d %s",
             char_type_name[c[i]->ctype],
             c[i]->symbol,
             abs(c[i]->pos[dim_y] - world.pc.pos[dim_y]),
             ((c[i]->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              "North" : "South"),
             abs(c[i]->pos[dim_x] - world.pc.pos[dim_x]),
             ((c[i]->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              "West" : "East"));
    if (count <= 13) {
      /* Handle the non-scrolling case right here. *
       * Scrolling in another function.            */
      mvprintw(i + 6, 19, " %-40s ", s[i]);
    }
  }

  if (count <= 13) {
    mvprintw(count + 6, 19, " %-40s ", "");
    mvprintw(count + 7, 19, " %-40s ", "Hit escape to continue.");
    while (getch() != 27 /* escape */)
      ;
  } else {
    mvprintw(19, 19, " %-40s ", "");
    mvprintw(20, 19, " %-40s ",
             "Arrows to scroll, escape to continue.");
    io_scroll_trainer_list(s, count);
  }

  free(s);
}

static void io_list_trainers()
{
  npc **c;
  uint32_t x, y, count;

  c = (npc **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = dynamic_cast <npc *> (world.cur_map->cmap[y][x]);
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  /* Display it */
  io_list_trainers_display(c, count);
  free(c);

  /* And redraw the map */
  io_display();
}

void io_pokemart()
{
  mvprintw(0, 0, "Welcome to the Pokemart.  Could I interest you in some Pokeballs?");
	// world.pc.bag[inv_revive] = 2;
	// world.pc.bag[inv_potion] = 2;
	// world.pc.bag[inv_pokeball] = 2;
  refresh();
  getch();
}

void io_pokemon_center()
{
  mvprintw(0, 0, "Welcome to the Pokemon Center.  How can Nurse Joy assist you?");
	// for (int i = 0; i < 6 && world.pc.buddy[i]; i++) {
	// 	world.pc.buddy[i]->heal(world.pc.buddy[i]->get_hp());
	// }
  refresh();
  getch();
}

// int io_inventory() {
// 	WINDOW *inventory;
// 	inventory = open_popup(MAP_Y - 4, 20, 3, 3);
// 	uint8_t close_bag = 0;
//
// 	int y, i, j;
//
// 	do {
// 		mvwprintw(inventory, 2, 2, "Inventory:");
// 		mvwprintw(inventory, 3, 3, "1|  Revives x%d", world.pc.bag[inv_revive]);
// 		mvwprintw(inventory, 4, 3, "2|  Potions x%d", world.pc.bag[inv_potion]);
// 		mvwprintw(inventory, 5, 3, "3|Pokeballs x%d", world.pc.bag[inv_pokeball]);
//
// 		mvwprintw(inventory, 7, 2, "[1|2|3]/[0]");
//
// 		switch(wgetch(inventory)) {
// 		case '1':
// 			/* Clear Menu */
// 			for (j = 2; j < 15; j++) {
// 				mvwprintw(inventory, j, 2, "%16s", "");
// 			}
// 			y = 2;
// 			mvwprintw(inventory, y++, 2, "Choose Pokemon:");
// 			/* List Unconcscious Pokemon */
// 			for (i = 0; i < 6 && world.pc.buddy[i]; i++) {
// 				if (world.pc.buddy[i]->get_chp() <= 0) {
// 					mvwprintw(inventory, y++, 3, "%d %12s", i, world.pc.buddy[i]->get_species());
// 					mvwprintw(inventory, y++, 5, "- L:%d | %d/%d", 
// 																					world.pc.buddy[i]->get_lvl(),
// 																					world.pc.buddy[i]->get_chp(),
// 																					world.pc.buddy[i]->get_hp());
// 				}
// 			}
// 			/* Display Valid Input Options*/
// 			switch(--i) {
// 				case 0:
// 				mvwprintw(inventory, y++, 2, "No 0hp Pokemon");
// 				mvwprintw(inventory, ++y, 2, "[0]");
// 				break;
// 				case 1:
// 				mvwprintw(inventory, ++y, 2, "[1]/[0]");
// 				break;
// 				case 2:
// 				mvwprintw(inventory, ++y, 2, "[1|2]/[0]");
// 				break;
// 				case 3:
// 				mvwprintw(inventory, ++y, 2, "[1|2|3]/[0]");
// 				break;
// 				case 4:
// 				mvwprintw(inventory, ++y, 2, "[1|2|3|4]/[0]");
// 				break;
// 				case 5:
// 				mvwprintw(inventory, ++y, 2, "[1|2|3|4|5]/[0]");
// 				break;
// 			}
//
// 			switch(wgetch(inventory)) {
// 			case '1':
// 				j = 0;
// 			case '2':
// 				j = 1;
// 			case '3':
// 				j = 2;
// 			case '4':
// 				j = 3;
// 			case '5':
// 				j = 4;
// 			case '6':
// 				j = 5;
// 				if (world.pc.buddy[i]) {
// 					world.pc.bag[inv_revive]--;
// 					world.pc.buddy[j]->heal(world.pc.buddy[j]->get_hp() / 2);
// 				}
// 				return j + 5;
// 				break;
// 			default:
// 				for (j = 2; j < 15; j++) {
// 					mvwprintw(inventory, j, 2, "%16s", "");
// 				}
// 				break;
// 			}
// 			break;
// 		case '2':
// 			/* Clear Menu */
// 			for (j = 2; j < 15; j++) {
// 				mvwprintw(inventory, j, 2, "%16s", "");
// 			}
// 			y = 2;
// 			mvwprintw(inventory, y++, 2, "Choose Pokemon:");
// 			/* List Unconcscious Pokemon */
// 			for (i = 0; i < 6 && world.pc.buddy[i]; i++) {
// 				if (world.pc.buddy[i]->get_chp() < world.pc.buddy[i]->get_hp()) {
// 					mvwprintw(inventory, y++, 3, "%d %12s", i, world.pc.buddy[i]->get_species());
// 					mvwprintw(inventory, y++, 5, "- L:%d | %d/%d", 
// 																					world.pc.buddy[i]->get_lvl(),
// 																					world.pc.buddy[i]->get_chp(),
// 																					world.pc.buddy[i]->get_hp());
// 				}
// 			}
// 			/* Display Valid Input Options*/
// 			switch(--i) {
// 				case 0:
// 				mvwprintw(inventory, y++, 2, "No Hurt Pokemon");
// 				mvwprintw(inventory, ++y, 2, "[0]");
// 				break;
// 				case 1:
// 				mvwprintw(inventory, ++y, 2, "[1]/[0]");
// 				break;
// 				case 2:
// 				mvwprintw(inventory, ++y, 2, "[1|2]/[0]");
// 				break;
// 				case 3:
// 				mvwprintw(inventory, ++y, 2, "[1|2|3]/[0]");
// 				break;
// 				case 4:
// 				mvwprintw(inventory, ++y, 2, "[1|2|3|4]/[0]");
// 				break;
// 				case 5:
// 				mvwprintw(inventory, ++y, 2, "[1|2|3|4|5]/[0]");
// 				break;
// 			}
//
// 			switch(wgetch(inventory)) {
// 			case '1':
// 				j = 0;
// 			case '2':
// 				j = 1;
// 			case '3':
// 				j = 2;
// 			case '4':
// 				j = 3;
// 			case '5':
// 				j = 4;
// 			case '6':
// 				j = 5;
// 				if (world.pc.buddy[j]) {
// 					world.pc.bag[inv_revive]--;
// 					world.pc.buddy[j]->heal(world.pc.buddy[j]->get_hp() / 2);
// 				}
// 				return j + 11;
// 				break;
// 			default:
// 				for (j = 2; j < 15; j++) {
// 					mvwprintw(inventory, j, 2, "%16s", "");
// 				}
// 				break;
// 			}
// 			break;
// 		case '3':
// 			mvwprintw(inventory, 9, 2, "Cannot Use Now");
// 			break;
// 		case '0':
// 			close_bag = 1;
// 			break;
// 		default:
// 			break;
// 		}
// 		wrefresh(inventory);
// 	} while (!close_bag);
//
// 	close_popup(inventory);
//
// 	return -1;
// }

// int mode, npc* n = NULL, pokemon* w = NULL

// int32_t battle_menu(int mode, npc* n = NULL, pokemon* w = NULL) {
//   WINDOW *battle_menu;
//   uint8_t end_battle = 0;
//   battle_menu = open_popup(MAP_Y - 2, MAP_X - 4, 2, 2);
//
//   pokemon& a = *(world.pc.buddy[0]);
//   int pc_lives = world.pc.num_buddies;
//   int pc_priority;
//   int pc_move = -1;
//
// 	pokemon& f = *(n->buddy[0]);
// 	int n_lives = n->num_buddies;
// 	if (mode) {
// 		f = *(w);
// 		n_lives = 1;
// 	}
//
// 	int victor;
//
//   int n_priority;
// 	int n_move = -1;
//
//   int turns = 0;
//   int ry = 2;
//   int ly = 2;
//   int i, j, y;
//
//   do {
// 		/* Clear Right Side */
//     ry = 2;
//     for (j = 2; j < 15; j++) {
//       mvwprintw(battle_menu, j, 56, "%19s", "");
//     }
//
// 		/* ###### Center Battle Stage ###### */
// 		if (!mode) {
// 			mvwprintw(battle_menu, 1, 31, "Enemy: %8s", char_type_name[n->ctype]);
// 		} else {
// 			mvwprintw(battle_menu, 1, 28, "Enemy: Wild %8s", f.get_species());
// 		}
//     mvwprintw(battle_menu, ry++, 62, "Turn #%d", ++turns);
//
//     mvwprintw(battle_menu, 4, 30, "%15s (%c)",
//                                     f.get_species(),
//                                     f.get_gender_string()[0]);
//     mvwprintw(battle_menu, 5, 40, "L:%d | %3d/%3d", f.get_lvl(), f.get_chp(), f.get_hp());
// 		mvwprintw(battle_menu, 6, 40, "< ");
// 		for (i = 0; i < n_lives; i++) {
// 			wprintw(battle_menu, "@ ");
// 		}
// 		wprintw(battle_menu, ">");
//
//     mvwprintw(battle_menu, 10, 18, "%15s (%c)",
//                                       a.get_species(),
//                                       a.get_gender_string()[0]);
//     mvwprintw(battle_menu, 11, 26, "L:%d | %3d/%3d", a.get_lvl(), a.get_chp(), a.get_hp());
// 		mvwprintw(battle_menu, 12, 26, "< ");
// 		for (i = 0; i < pc_lives; i++) {
// 			wprintw(battle_menu, "@ ");
// 		}
// 		wprintw(battle_menu, ">");
//
// 		if (!mode) {
// 			mvwprintw(battle_menu, 15, 18, "Options: [Z] Fight | [X] Bag | [V] Switch");
// 		} else {
// 			mvwprintw(battle_menu, 15, 13, "Options: [Z] Fight | [X] Bag | [C] Run | [V] Switch");
// 		}
//     // mvwprintw(battle_menu, 17, 29, "Press [Esc] To Exit");
//
// 		/* ##### PC Actions ##### */
// 		pc_move = -1;
// 		while (pc_move < 0) {
// 			ly = 2;
// 			if (a.get_chp() <= 0) {
// 				if (pc_lives > 0) {
// 					ungetch('V');
// 				} else {
// 					end_battle = 1;
// 					pc_move = 100;
// 					break;
// 				}
// 			}
// 			switch(wgetch(battle_menu)) {
// 			case 27:
// 				end_battle = 1;
// 				pc_move = 100;
// 				ungetch('>');
// 				break;
// 			case 'Z':
// 			case 'z':
// 				pc_priority = INT_MAX;
// 				mvwprintw(battle_menu, ry++, 56, "Your action: Fight");
//
// 				mvwprintw(battle_menu, ly++, 3, "Attacks:");
//
// 				for (i = 0; i < 4; i++) {
// 					if (std::string(a.get_move(i)).length() == 0) {
// 						break;
// 					}
// 					mvwprintw(battle_menu, ly++, 4, "%d|%s", i+1, a.get_move(i));
// 				}
// 				if (i == 1) {
// 					mvwprintw(battle_menu, ly++, 2, "[1]/[0]");
// 					switch(wgetch(battle_menu)) {
// 					 case '1':
// 						pc_priority = a.move_priority(0);
// 						pc_move = 0;
// 						break;
// 					 case '0':
// 					 	ly--;
// 						ry--;
// 						break;
// 					 default:
// 					 	ly--;
// 						ry--;
// 						break;
// 					}
// 				} else if (i == 2) {
// 					mvwprintw(battle_menu, ly++, 2, "[1|2]/[0]");
// 					switch(wgetch(battle_menu)) {
// 					case '1':
// 						pc_priority = a.move_priority(0);
// 						pc_move = 0;
// 						break;
// 					case '2':
// 						pc_priority = a.move_priority(1);
// 						pc_move = 1;
// 						break;
// 					case '0':
// 					 	ly--;
// 						ry--;
// 						break;
// 					}
// 				} else if (i == 3) {
// 					mvwprintw(battle_menu, ly++, 2, "[1|2|3]/[0]");
// 					switch(wgetch(battle_menu)) {
// 					case '1':
// 						pc_priority = a.move_priority(0);
// 						pc_move = 0;
// 						break;
// 					case '2':
// 						pc_priority = a.move_priority(1);
// 						pc_move = 1;
// 						break;
// 					case '3':
// 						pc_priority = a.move_priority(2);
// 						pc_move = 2;
// 						break;
// 					case '0':
// 					 	ly--;
// 						ry--;
// 						break;
// 					}
// 				}
//
// 				break;
//
// 			case 'X':
// 			case 'x':
// 				pc_priority = INT_MAX;
// 				mvwprintw(battle_menu, ry++, 56, "Your action: Bag  ");
//
// 				mvwprintw(battle_menu, 2, 2, "Inventory:");
// 				mvwprintw(battle_menu, 3, 3, "1|  Revives x%d", world.pc.bag[inv_revive]);
// 				mvwprintw(battle_menu, 4, 3, "2|  Potions x%d", world.pc.bag[inv_potion]);
// 				mvwprintw(battle_menu, 5, 3, "3|Pokeballs x%d", world.pc.bag[inv_pokeball]);
//
// 				mvwprintw(battle_menu, 7, 2, "[1|2|3]/[0]");
//
// 				switch(wgetch(battle_menu)) {
// 				case '1':
// 					pc_move = 5;
// 					mvwprintw(battle_menu, ry++, 58, "Using Revive");
// 					/* Clear Left Side */
// 					for (j = 2; j < 15; j++) {
// 						mvwprintw(battle_menu, j, 2, "%16s", "");
// 					}
// 					y = 2;
// 					mvwprintw(battle_menu, y++, 2, "Choose Pokemon:");
//           /* List Unconcscious Pokemon */
// 					for (i = 0; i < 6 && world.pc.buddy[i]; i++) {
// 						if (world.pc.buddy[i]->get_chp() <= 0) {
// 							mvwprintw(battle_menu, y++, 3, "%d %12s", i, world.pc.buddy[i]->get_species());
// 							mvwprintw(battle_menu, y++, 5, "- L:%d | %d/%d", 
// 																							world.pc.buddy[i]->get_lvl(),
// 																							world.pc.buddy[i]->get_chp(),
// 																							world.pc.buddy[i]->get_hp());
// 						}
// 					}
// 					/* Display Valid Input Options*/
//           switch(--i) {
//            case 0:
// 						mvwprintw(battle_menu, y++, 2, "No 0hp Pokemon");
// 						mvwprintw(battle_menu, ++y, 2, "[0]");
//             break;
//            case 1:
// 						mvwprintw(battle_menu, ++y, 2, "[1]/[0]");
//             break;
//            case 2:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2]/[0]");
//             break;
//            case 3:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3]/[0]");
//             break;
//            case 4:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3|4]/[0]");
//             break;
//            case 5:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3|4|5]/[0]");
//             break;
//           }
//
// 					switch(wgetch(battle_menu)) {
// 					case '1':
// 						j = 0;
// 						pc_move += j;
// 						break;
// 					case '2':
// 						j = 1;
// 						pc_move += j;
// 						break;
// 					case '3':
// 						j = 2;
// 						pc_move += j;
// 						break;
// 					case '4':
// 						j = 3;
// 						pc_move += j;
// 						break;
// 					case '5':
// 						j = 4;
// 						pc_move += j;
// 						break;
// 					case '6':
// 						j = 5;
// 						pc_move += j;
// 						break;
// 					default:
// 						pc_move = -1;
// 						break;
// 					}
// 					break;
// 				case '2':
// 					pc_move = 11;
// 					mvwprintw(battle_menu, ry++, 58, "Using Potion");
//
// 					/* Clear Left Side */
// 					for (j = 2; j < 15; j++) {
// 						mvwprintw(battle_menu, j, 2, "%16s", "");
// 					}
//           /* List Pokemon */
// 					y = 2;
// 					mvwprintw(battle_menu, y++, 2, "Choose Pokemon:");
// 					for (i = 0; i < 6 && world.pc.buddy[i]; i++) {
// 						if (world.pc.buddy[i]->get_chp() < world.pc.buddy[i]->get_hp()) {
// 							mvwprintw(battle_menu, y++, 3, "%d %12s", i+1, world.pc.buddy[i]->get_species());
// 							mvwprintw(battle_menu, y++, 5, "- L:%d | %d/%d", 
// 																							world.pc.buddy[i]->get_lvl(),
// 																							world.pc.buddy[i]->get_chp(),
// 																							world.pc.buddy[i]->get_hp());
// 						}
// 					}
//           /* Show Input Options */
// 					switch(i) {
//            case 0:
// 						mvwprintw(battle_menu, y++, 2, "No Hurt Pokemon");
// 						mvwprintw(battle_menu, ++y, 2, "[0]");
//             break;
//            case 1:
// 						mvwprintw(battle_menu, ++y, 2, "[1]/[0]");
//             break;
//            case 2:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2]/[0]");
//             break;
//            case 3:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3]/[0]");
//             break;
//            case 4:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3|4]/[0]");
//             break;
//            case 5:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3|4|5]/[0]");
//             break;
//            case 6:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3|4|5|6]/[0]");
//             break;
//           }
//
// 					switch(wgetch(battle_menu)) {
// 					case '1':
// 						j = 0;
// 						pc_move += j;
// 						break;
// 					case '2':
// 						j = 1;
// 						pc_move += j;
// 						break;
// 					case '3':
// 						j = 2;
// 						pc_move += j;
// 						break;
// 					case '4':
// 						j = 3;
// 						pc_move += j;
// 						break;
// 					case '5':
// 						j = 4;
// 						pc_move += j;
// 						break;
// 					case '6':
// 						j = 5;
// 						pc_move += j;
// 						break;
// 					default:
// 						pc_move = -1;
// 						break;
// 					}
// 					break;
// 				case '3':
// 					if (!mode) {
// 						pc_move = -1;
// 						break;
// 					} else {
// 						pc_move = 18;
// 						mvwprintw(battle_menu, ry++, 58, "Using Pokeball");
// 					}
// 					break;
// 				case '0':
// 					ry--;
// 					break;
// 				}
// 				break;
// 			case 'C':
// 			case 'c':
// 				if (!mode) {
// 					pc_move = -1;
// 					break;
// 				} else {
// 					pc_priority = INT_MAX;
// 					pc_move = 20;
// 					mvwprintw(battle_menu, ry++, 56, "Your action: Run  ");
// 					mvwprintw(battle_menu, 16, 32, "Flee: ");
// 				}
//
// 				break;
// 			case 'V':
// 			case 'v':
// 				pc_priority = INT_MAX;
// 				pc_move = 30;
// 				mvwprintw(battle_menu, ry++, 56, "Your action: Swap ");
//         /* List Pokemon */
// 				y = 2;
// 				mvwprintw(battle_menu, y++, 2, "Swap Pokemon:");
// 				for (i = 1; i < 6 && world.pc.buddy[i] && world.pc.buddy[i]->get_chp() > 0; i++) {
// 					mvwprintw(battle_menu, y++, 3, "%d %12s", i, world.pc.buddy[i]->get_species());
// 					mvwprintw(battle_menu, y++, 5, "- L:%d | %d/%d", 
// 																					world.pc.buddy[i]->get_lvl(),
// 																					world.pc.buddy[i]->get_chp(),
// 																					world.pc.buddy[i]->get_hp());
// 				}
//         /* Show Valid Input Options*/
// 				switch(i) {
//            case 0:
// 						mvwprintw(battle_menu, y++, 2, "No Hurt Pokemon");
// 						mvwprintw(battle_menu, ++y, 2, "[0]");
//             break;
//            case 1:
// 						mvwprintw(battle_menu, ++y, 2, "[1]/[0]");
//             break;
//            case 2:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2]/[0]");
//             break;
//            case 3:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3]/[0]");
//             break;
//            case 4:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3|4]/[0]");
//             break;
//            case 5:
// 						mvwprintw(battle_menu, ++y, 2, "[1|2|3|4|5]/[0]");
//             break;
//            default:
//             break;
//           }
//
// 				switch(wgetch(battle_menu)) {
// 				case '1':
// 					j = 1;
// 					pc_move += j;
// 					break;
// 				case '2':
// 					j = 2;
// 					pc_move += j;
// 					break;
// 				case '3':
// 					j = 3;
// 					pc_move += j;
// 					break;
// 				case '4':
// 					j = 4;
// 					pc_move += j;
// 					break;
// 				case '5':
// 					j = 5;
// 					pc_move += j;
// 					break;
// 				case '0':
// 					pc_move = -1;
// 					ry--;
// 					break;
// 				}
// 			}
// 			/* Clear Left Side */
// 			for (j = 2; j < 15; j++) {
// 				mvwprintw(battle_menu, j, 2, "%16s", "");
// 			}
// 		}
//
// 		n_move = -1;
// 		if (f.get_chp() > 0) {
// 			for (i = 0; i < 4; i++) {
// 				if (std::string(f.get_move(i)).length() == 0) {
// 					break;
// 				}
// 			}
// 			n_move = (rand() % i);
// 			n_priority = f.move_priority(n_move);
// 		} else if (n_lives > 0) {
// 			n_priority = INT_MAX;
// 			n_move = 30;
// 		} else {
// 			if (!mode) 	{ n->defeated = 1; }
// 			else				{ delete w; }
// 			end_battle = 1;
// 			break;
// 		}
//
//     mvwprintw(battle_menu, ry++, 56, "Opp. Action: Fight");
// 		if (n_priority == pc_priority) {
// 			n_priority = f.get_speed();
// 			pc_priority = a.get_speed();
// 			if (n_priority == pc_priority) {
// 				(rand() % 2) ? n_priority++ : n_priority--;
// 			}
// 		}
//     if (n_priority > pc_priority) {
//       mvwprintw(battle_menu, ry++, 60, "Opp. Acts First");
// 			ry++;
// 			if (n_move < 5) {
// 				mvwprintw(battle_menu, ry++, 57, "%10s | %2d%%", 
//                                           f.get_move(n_move), 
//                                           (f.move_accuracy(n_move) < INT_MAX)
//                                             ? f.move_accuracy(n_move) : 0);
// 				mvwprintw(battle_menu, ry++, 60, "Attack: %d dmg", f.attack(n_move, a));
// 				// while (wgetch(battle_menu) != '>') 
// 				// 	{ wrefresh(battle_menu); }
// 				if (a.get_chp() <= 0) {
// 					pc_lives--;
// 				}
// 			} else if (n_move == 30) {
// 				n->swap();
// 				f = *(n->buddy[0]);
// 			}
// 			if (a.get_chp() > 0) {
// 				if (pc_move < 5) {
//           mvwprintw(battle_menu, ry++, 56, "%11s | %2d%%", 
//                                             a.get_move(pc_move), 
//                                             (a.move_accuracy(pc_move) < INT_MAX)
//                                               ? a.move_accuracy(pc_move) : 0);
// 					mvwprintw(battle_menu, ry++, 60, "Attack: %d dmg", a.attack(pc_move, f));
// 					// while (wgetch(battle_menu) != '>') 
// 					// 	{ wrefresh(battle_menu); }
// 					if (f.get_chp() <= 0) {
// 						n_lives--;
// 						if (mode) { delete w; }
// 					}
// 				} else if (pc_move < 11) {
// 					j = pc_move - 5;
// 					if (world.pc.buddy[j] && world.pc.buddy[j]->get_chp() <= 0) {
// 						world.pc.bag[inv_revive]--;
// 						mvwprintw(battle_menu, ry++, 60, "Revive : %d", 
// 														world.pc.buddy[j]->heal(world.pc.buddy[j]->get_hp() / 2));
// 						pc_lives++;
// 					}
// 				} else if (pc_move < 17) {
// 					j = pc_move - 11;
// 					if (world.pc.buddy[j]) {
// 						//  && (world.pc.buddy[j]->get_chp() < world.pc.buddy[j]->get_hp())
// 						world.pc.bag[inv_potion]--;
// 						mvwprintw(battle_menu, ry++, 60, "HP +20 : %d", 
// 														world.pc.buddy[j]->heal(20));
// 						// world.pc.buddy[j]->heal(20);
// 					}
// 				} else if (pc_move == 18) {
// 					mvwprintw(battle_menu, 16, 30, "Capture: ");
// 					if (world.pc.num_buddies < 6) {
// 						world.pc.buddy[world.pc.num_buddies++] = &f;
// 						mvwprintw(battle_menu, 16, 39, "Success!");
// 						world.pc.bag[inv_pokeball]--;
//
// 						// while (wgetch(battle_menu) != '>') 
// 						// 	{ wrefresh(battle_menu); }
// 						end_battle = 1;
// 					} else {
// 						mvwprintw(battle_menu, 16, 39, "Failed :(");
// 						delete w;
// 					}
// 				} else if (pc_move == 20) {
// 					if (rand() % 100 < FLEE_CHANCE) {
// 						end_battle = 1;
// 						mvwprintw(battle_menu, 16, 38, "Success");
// 						io_queue_message("You fled the battle.");
// 						if (mode) { delete w; }
// 					} else {
// 						mvwprintw(battle_menu, 16, 38, "Fail");
// 						ry--;
// 					}
// 				} else if (pc_move >= 30) {
// 					j = pc_move - 30;
// 					if (j >= 1 && world.pc.buddy[j] && world.pc.buddy[j]->get_chp() > 0) {
// 						pokemon* out = world.pc.buddy[0];
// 						world.pc.buddy[0] = world.pc.buddy[j];
// 						world.pc.buddy[j] = out;
// 						mvwprintw(battle_menu, ry++, 58, "%s, I choose you!", world.pc.buddy[0]->get_species());
// 					}
// 					a = *(world.pc.buddy[0]);
// 				}
// 			}
//     } else {
//       mvwprintw(battle_menu, ry++, 61, "You Act First");
// 			if (pc_move < 5) {
//         mvwprintw(battle_menu, ry++, 57, "%10s | %2d%%", 
//                                           a.get_move(pc_move), 
//                                           (a.move_accuracy(pc_move) < INT_MAX)
//                                             ? a.move_accuracy(pc_move) : 0);
// 				mvwprintw(battle_menu, ry++, 60, "Attack: %d dmg", a.attack(pc_move, f));
// 				// while (wgetch(battle_menu) != '>') 
// 				// 	{ wrefresh(battle_menu); }
// 				if (f.get_chp() <= 0) {
// 					if (mode) { delete w; }
// 					n_lives--;
// 				}
// 			} else if (pc_move < 11) {
// 				j = pc_move - 5;
// 				if (world.pc.buddy[j] && world.pc.buddy[j]->get_chp() <= 0) {
// 					world.pc.bag[inv_revive]--;
// 					world.pc.buddy[j]->heal(world.pc.buddy[j]->get_hp() / 2);
// 					pc_lives++;
// 				}
// 			} else if (pc_move < 17) {
// 				j = pc_move - 11;
// 				if (world.pc.buddy[j]) {
// 				//  && (world.pc.buddy[j]->get_chp() < world.pc.buddy[j]->get_hp()
// 					world.pc.bag[inv_potion]--;
// 					mvwprintw(battle_menu, ry++, 60, "HP +20 : %d", 
// 														world.pc.buddy[j]->heal(20));
// 				}
// 			} else if (pc_move == 18) {
// 				mvwprintw(battle_menu, 16, 30, "Capture: ");
// 				if (world.pc.num_buddies < 6) {
// 					world.pc.buddy[world.pc.num_buddies++] = &f;
// 					mvwprintw(battle_menu, 16, 39, "Success!");
// 					world.pc.bag[inv_pokeball]--;
//
// 					// while (wgetch(battle_menu) != '>') 
// 					// 	{ wrefresh(battle_menu); }
// 					end_battle = 1;
// 				} else {
// 					mvwprintw(battle_menu, 16, 39, "Failed :(");
// 					delete w;
// 				}
// 			} else if (pc_move == 20) {
// 				if (rand() % 100 < FLEE_CHANCE) {
// 					end_battle = 1;
// 					mvwprintw(battle_menu, 16, 38, "Success");
// 					io_queue_message("You fled the battle.");
// 					if (mode) { delete w; }
// 				} else {
// 					mvwprintw(battle_menu, 16, 38, "Fail");
// 					ry--;
// 				}
// 			} else if (pc_move >= 30) {
// 				j = pc_move - 30;
// 				if (j >= 1 && world.pc.buddy[j] && world.pc.buddy[j]->get_chp() > 0) {
// 					pokemon* out = world.pc.buddy[0];
// 					world.pc.buddy[0] = world.pc.buddy[j];
// 					world.pc.buddy[j] = out;
// 					mvwprintw(battle_menu, ry++, 58, "%s, I choose you!", world.pc.buddy[0]->get_species());
// 				}
// 				a = *(world.pc.buddy[0]);
//
// 			}
// 			if (f.get_chp() > 0) {
// 				if (n_move < 5) {
// 					mvwprintw(battle_menu, ry++, 57, "%10s | %2d%%", 
// 																						f.get_move(n_move), 
// 																						(f.move_accuracy(n_move) < INT_MAX)
// 																							? f.move_accuracy(n_move) : 0);
// 					mvwprintw(battle_menu, ry++, 60, "Attack: %d dmg", f.attack(n_move, a));
// 					// while (wgetch(battle_menu) != '>') 
// 					// 	{ wrefresh(battle_menu); }
// 					if (a.get_chp() <= 0) {
// 						if (mode) { delete w; }
// 						pc_lives--;
// 					}
// 				} else if (n_move == 30) {
// 					n->swap();
// 					f = *(n->buddy[0]);
// 				}
// 			}
//     }
//
//     if (pc_lives == 0) {
//       end_battle = 1;
//     }
//     if (n_lives == 0) {
//       if (!mode) { n->defeated = 1; }
// 			else			 { delete w; }
//       end_battle = 1;
//     }
//     mvwprintw(battle_menu, ++ry, 64, "[ ] Cont");
//     while (wgetch(battle_menu) != ' ') 
// 			{ wrefresh(battle_menu); }
//     // wrefresh(battle_menu);
//   } while (!end_battle);
//
//   close_popup(battle_menu);
//
// 	victor = pc_lives;
//   return victor;
// }

void io_battle(character *aggressor, character *defender)
{
  std::string s;
  npc *n = (npc *) ((aggressor == &world.pc) ? defender : aggressor);
  // int i;
  if (aggressor == &world.pc) {
    io_queue_message("You ask Trainer %c to Battle!", n->symbol);
  } else {
    io_queue_message("Trainer %c wants to Battle!", n->symbol);
  }

  /* s = std::string(1, n->symbol) + ": ";
  // io_queue_message("%s: ", char_type_name[n->ctype]);
  if (!n->buddy[1]) {
    s += "My pokemon is " + std::string(n->buddy[0]->get_species());
  } else {
    s += "My pokemon are " + std::string(n->buddy[0]->get_species());
  }

  for (i = 1; i < 6 && n->buddy[i]; i++) {
    s += ", ";
    if (i == 4 || !n->buddy[i + 1]) {
      s += "and ";
    }
    s += n->buddy[i]->get_species();
  }
    
  s += ".";

  io_queue_message("%s", s.c_str()); */
	// if (world.pc.buddy[0]->get_chp() <= 0) {
	// 	io_queue_message("You're out of Pokemon!");
	// 	io_print_message_queue(0, 0);
	// 	return;
	// }
  io_print_message_queue(0, 0);

  // if (battle_menu(0, n)) {
	// }
		if (n->ctype == char_hiker || n->ctype == char_rival) {
			n->mtype = move_wander;
		}

  // n->defeated = 1;
}

uint32_t move_pc_dir(uint32_t input, pair_t dest)
{
  dest[dim_y] = world.pc.pos[dim_y];
  dest[dim_x] = world.pc.pos[dim_x];

  switch (input) {
  case 1:
  case 2:
  case 3:
    dest[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    dest[dim_y]--;
    break;
  }
  switch (input) {
  case 1:
  case 4:
  case 7:
    dest[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    dest[dim_x]++;
    break;
  case '>':
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_mart) {
      io_pokemart();
    }
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_center) {
      io_pokemon_center();
    }
    break;
  }

  if (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) {
    if (dynamic_cast<npc *> (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) &&
        ((npc *) world.cur_map->cmap[dest[dim_y]][dest[dim_x]])->defeated) {
      // Some kind of greeting here would be nice
      return 1;
    } else if ((dynamic_cast<npc *>
                (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]))) {
      io_battle(&world.pc, world.cur_map->cmap[dest[dim_y]][dest[dim_x]]);
      // Not actually moving, so set dest back to PC position
      dest[dim_x] = world.pc.pos[dim_x];
      dest[dim_y] = world.pc.pos[dim_y];
    }
  }
  
  if (move_cost[char_pc][world.cur_map->map[dest[dim_y]][dest[dim_x]]] ==
      DIJKSTRA_PATH_MAX) {
    return 1;
  }

  return 0;
}

void io_teleport_world(pair_t dest)
{
  /* mvscanw documentation is unclear about return values.  I believe *
   * that the return value works the same way as scanf, but instead   *
   * of counting on that, we'll initialize x and y to out of bounds   *
   * values and accept their updates only if in range.                */
  int x = INT_MAX, y = INT_MAX;
  
  world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = NULL;

  echo();
  curs_set(1);
  do {
    mvprintw(0, 0, "Enter x [-200, 200]:           ");
    refresh();
    mvscanw(0, 21, (char *) "%d", &x);
  } while (x < -200 || x > 200);
  do {
    mvprintw(0, 0, "Enter y [-200, 200]:          ");
    refresh();
    mvscanw(0, 21, (char *) "%d", &y);
  } while (y < -200 || y > 200);

  refresh();
  noecho();
  curs_set(0);

  x += 200;
  y += 200;

  world.cur_idx[dim_x] = x;
  world.cur_idx[dim_y] = y;

  new_map(1);
  io_teleport_pc(dest);
}

void io_handle_input(pair_t dest)
{
  uint32_t turn_not_consumed;
  int key;

  do {
    switch (key = getch()) {
    case '7':
    case 'y':
    case KEY_HOME:
      turn_not_consumed = move_pc_dir(7, dest);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      turn_not_consumed = move_pc_dir(8, dest);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      turn_not_consumed = move_pc_dir(9, dest);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      turn_not_consumed = move_pc_dir(6, dest);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      turn_not_consumed = move_pc_dir(3, dest);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      turn_not_consumed = move_pc_dir(2, dest);
      break;
    case '1':
    case 'b':
    case KEY_END:
      turn_not_consumed = move_pc_dir(1, dest);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      turn_not_consumed = move_pc_dir(4, dest);
      break;
    case '5':
    case ' ':
    case '.':
    case KEY_B2:
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    case '>':
      turn_not_consumed = move_pc_dir('>', dest);
      break;
    case 'Q':
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      world.quit = 1;
      turn_not_consumed = 0;
      break;
      break;
    case 't':
      io_list_trainers();
      turn_not_consumed = 1;
      break;
		case 'B':
			io_inventory();
			turn_not_consumed = 1;
			io_display();
			break;
    case 'p':
      /* Teleport the PC to a random place in the map.              */
      io_teleport_pc(dest);
      turn_not_consumed = 0;
      break;
   case 'f':
      /* Fly to any map in the world.                                */
      io_teleport_world(dest);
      turn_not_consumed = 0;
      break;    
    case 'q':
      /* Demonstrate use of the message queue.  You can use this for *
       * printf()-style debugging (though gdb is probably a better   *
       * option.  Not that it matters, but using this command will   *
       * waste a turn.  Set turn_not_consumed to 1 and you should be *
       * able to figure out why I did it that way.                   */
      io_queue_message("This is the first message.");
      io_queue_message("Since there are multiple messages, "
                       "you will see \"more\" prompts.");
      io_queue_message("You can use any key to advance through messages.");
      io_queue_message("Normal gameplay will not resume until the queue "
                       "is empty.");
      io_queue_message("Long lines will be truncated, not wrapped.");
      io_queue_message("io_queue_message() is variadic and handles "
                       "all printf() conversion specifiers.");
      io_queue_message("Did you see %s?", "what I did there");
      io_queue_message("When the last message is displayed, there will "
                       "be no \"more\" prompt.");
      io_queue_message("Have fun!  And happy printing!");
      io_queue_message("Oh!  And use 'Q' to quit!");

      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      turn_not_consumed = 1;
    }
    refresh();
  } while (turn_not_consumed);
}

void io_encounter_pokemon()
{
  pokemon *p;

  p = new pokemon();

  io_queue_message("%s%s%s: HP:%d ATK:%d DEF:%d SPATK:%d SPDEF:%d SPEED:%d %s",
                   p->is_shiny() ? "*" : "", p->get_species(),
                   p->is_shiny() ? "*" : "", p->get_hp(), p->get_atk(),
                   p->get_def(), p->get_spatk(), p->get_spdef(),
                   p->get_speed(), p->get_gender_string());
  io_queue_message("%s's moves: %s %s", p->get_species(),
                   p->get_move(0), p->get_move(1));

	battle_menu(1, NULL, p);

  // Later on, don't delete if captured
  // delete p;
}

void io_choose_starter()
{
  class pokemon *choice[3];
  int i;
  bool again = true;
  
  choice[0] = new class pokemon();
  choice[1] = new class pokemon();
  choice[2] = new class pokemon();

  echo();
  curs_set(1);
  do {
    mvprintw( 4, 20, "Before you are three Pokemon, each of");
    mvprintw( 5, 20, "which wants absolutely nothing more");
    mvprintw( 6, 20, "than to be your best buddy forever.");
    mvprintw( 8, 20, "Unfortunately for them, you may only");
    mvprintw( 9, 20, "pick one.  Choose wisely.");
    mvprintw(11, 20, "   1) %s", choice[0]->get_species());
    mvprintw(12, 20, "   2) %s", choice[1]->get_species());
    mvprintw(13, 20, "   3) %s", choice[2]->get_species());
    mvprintw(15, 20, "Enter 1, 2, or 3: ");

    refresh();
    i = getch();

    if (i == '1' || i == '2' || i == '3') {
      world.pc.buddy[0] = choice[(i - '0') - 1];
      world.pc.num_buddies = 1;
      delete choice[(i - '0') % 3];
      delete choice[((i - '0') + 1) % 3];
      again = false;
    }
  } while (again);
  noecho();
  curs_set(0);
}
