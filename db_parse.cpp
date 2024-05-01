#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <climits>

#include "db_parse.h"

static char *next_token(char *start, char delim)
{
  int i;
  static char *s;

  if (start) {
    s = start;
  }

  start = s;

  for (i = 0; s[i] && s[i] != delim; i++)
    ;
  s[i] = '\0';
  s = s + i + 1;

  return start;
}

/* We can't print a "null integer", so it takes an annoying amount of code *
 * to check for INT_MAX and then print "", otherwise print the integer     *
 * value.  This function converts ints to strings only if they do not have *
 * the value INT_MAX and returns the string, so no extra work needs to be  *
 * done where we print (except calling this function).                     *
 *                                                                         *
 * Needs an internal array because arguments are processed before the      *
 * function call, so if we returned the same pointer over and over again,  *
 * we'd print a bunch of whatever the final value processed was.           */
static const char *i2s(int i)
{
  static int next = 0;
  static char s[20][12];

  if (next == 20) {
    next = 0;
  }
  
  if (i == INT_MAX) {
    s[next][0] = '\0';
  } else {
    sprintf(s[next], "%d", i);
  }

  return s[next++];
}

pokemon_move_db pokemon_moves[528239];
pokemon_db pokemon[1093];
char *types[19];
move_db moves[845];
pokemon_species_db species[899];
experience_db experience[601];
pokemon_stats_db pokemon_stats[6553];
stats_db stats[9];
pokemon_types_db pokemon_types[1676];

void db_parse(bool print)
{
  FILE *f;
  char line[800];
  int i;
  char *tmp;
  struct stat buf;
  char *prefix;
  int prefix_len;
  int j;
  int count;
  
  i = (strlen(getenv("HOME")) +
       strlen("/.poke327/pokedex/pokedex/data/csv/") + 1);
  prefix = (char *) malloc(i);
  strcpy(prefix, getenv("HOME"));
  strcat(prefix, "/.poke327/pokedex/pokedex/data/csv/");

  if (stat(prefix, &buf)) {
    free(prefix);
    prefix = NULL;
  }

  if (!prefix && !stat("/share/cs327", &buf)) {
    prefix = strdup("/share/cs327/pokedex/pokedex/data/csv/");
  } else if (!prefix) {
    // Your third location goes here, if needed.
    // prefix is freed later, so be sure you malloc it
  }

  //No error checking on file load from here on out.  Missing
  //files are "user error".
  prefix_len = strlen(prefix);

  prefix = (char *) realloc(prefix, prefix_len + strlen("pokemon.csv") + 1);
  strcpy(prefix + prefix_len, "pokemon.csv");
  
  f = fopen(prefix, "r");

  //No null byte copied here, so prefix is not technically a string anymore.
  prefix = (char *) realloc(prefix, prefix_len + 1);

  fgets(line, 80, f);
  
  for (i = 1; i < 1093; i++) {
    fgets(line, 80, f);
    pokemon[i].id = atoi(next_token(line, ','));
    strncpy(pokemon[i].identifier, next_token(NULL, ','), 30);
    pokemon[i].species_id = atoi(next_token(NULL, ','));
    pokemon[i].height = atoi(next_token(NULL, ','));
    pokemon[i].weight = atoi(next_token(NULL, ','));
    pokemon[i].base_experience = atoi(next_token(NULL, ','));
    pokemon[i].order = atoi(next_token(NULL, ','));
    pokemon[i].is_default = atoi(next_token(NULL, ','));
  }  

  fclose(f);
  
  if (print) {
    f = fopen("pokemon.csv", "w");
    for (i = 1; i < 1093; i++) {
      fprintf(f, "%s,%s,%s,%s,%s,%s,%s,%s\n",
              i2s(pokemon[i].id),
              pokemon[i].identifier,
              i2s(pokemon[i].species_id),
              i2s(pokemon[i].height),
              i2s(pokemon[i].weight),
              i2s(pokemon[i].base_experience),
              i2s(pokemon[i].order),
              i2s(pokemon[i].is_default));
    }
    fclose(f);
  }


  prefix = (char *) realloc(prefix, prefix_len + strlen("moves.csv") + 1);
  strcpy(prefix + prefix_len, "moves.csv");
  
  f = fopen(prefix, "r");

  //No null byte copied here, so prefix is not technically a string anymore.
  prefix = (char *) realloc(prefix, prefix_len + 1);

  fgets(line, 800, f);
  
  for (i = 1; i < 845; i++) {
    fgets(line, 800, f);
    moves[i].id = atoi((tmp = next_token(line, ',')));
    strcpy(moves[i].identifier, (tmp = next_token(NULL, ',')));
    tmp = next_token(NULL, ',');
    moves[i].generation_id = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].type_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].power =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].pp =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].accuracy =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].priority =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].target_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].damage_class_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].effect_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].effect_chance =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].contest_type_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].contest_effect_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    moves[i].super_contest_effect_id = (*tmp != '\n') ? atoi(tmp) : INT_MAX;
  }

  fclose(f);
  
  if (print) {
    f = fopen("moves.csv", "w");
    for (i = 1; i < 845; i++) {
      fprintf(f, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
              i2s(moves[i].id),
              moves[i].identifier,
              i2s(moves[i].generation_id),
              i2s(moves[i].type_id),
              i2s(moves[i].power),
              i2s(moves[i].pp),
              i2s(moves[i].accuracy),
              i2s(moves[i].priority),
              i2s(moves[i].target_id),
              i2s(moves[i].damage_class_id),
              i2s(moves[i].effect_id),
              i2s(moves[i].effect_chance),
              i2s(moves[i].contest_type_id),
              i2s(moves[i].contest_effect_id),
              i2s(moves[i].super_contest_effect_id));
    }
    fclose(f);
  }

  prefix = (char *) realloc(prefix,
                            prefix_len + strlen("pokemon_moves.csv") + 1);
  strcpy(prefix + prefix_len, "pokemon_moves.csv");
  
  f = fopen(prefix, "r");

  //No null byte copied here, so prefix is not technically a string anymore.
  prefix = (char *) realloc(prefix, prefix_len + 1);

  fgets(line, 800, f);
  
  for (i = 1; i < 528239; i++) {
    fgets(line, 800, f);
    tmp = next_token(line, ',');
    pokemon_moves[i].pokemon_id = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    pokemon_moves[i].version_group_id = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    pokemon_moves[i].move_id = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    pokemon_moves[i].pokemon_move_method_id = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    pokemon_moves[i].level = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    pokemon_moves[i].order = (*tmp != '\n') ? atoi(tmp) : INT_MAX;
  }

  fclose(f);

  if (print) {
    f = fopen("pokemon_moves.csv", "w");
    for (i = 1; i < 528239; i++) {
      fprintf(f, "%s,%s,%s,%s,%s,%s\n",
              i2s(pokemon_moves[i].pokemon_id),
              i2s(pokemon_moves[i].version_group_id),
              i2s(pokemon_moves[i].move_id),
              i2s(pokemon_moves[i].pokemon_move_method_id),
              i2s(pokemon_moves[i].level),
              i2s(pokemon_moves[i].order));
    }
    fclose(f);
  }

  prefix = (char *) realloc(prefix,
                            prefix_len + strlen("pokemon_species.csv") + 1);
  strcpy(prefix + prefix_len, "pokemon_species.csv");
  
  f = fopen(prefix, "r");

  //No null byte copied here, so prefix is not technically a string anymore.
  prefix = (char *) realloc(prefix, prefix_len + 1);

  fgets(line, 800, f);
  
  for (i = 1; i < 899; i++) {
    fgets(line, 800, f);
    species[i].id = atoi((tmp = next_token(line, ',')));
    strcpy(species[i].identifier, (tmp = next_token(NULL, ',')));
    tmp = next_token(NULL, ',');
    species[i].generation_id = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].evolves_from_species_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].evolution_chain_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].color_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].shape_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].habitat_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].gender_rate =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].capture_rate =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].base_happiness =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].is_baby =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].hatch_counter =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].has_gender_differences =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].growth_rate_id =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].forms_switchable =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].is_legendary =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].is_mythical =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].order =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    species[i].conquest_order = (*tmp != '\n') ? atoi(tmp) : INT_MAX;
  }

  fclose(f);

  if (print) {
    f = fopen("pokemon_species.csv", "w");
    for (i = 1; i < 899; i++) {
      fprintf(f,
              "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
              i2s(species[i].id),
              species[i].identifier,
              i2s(species[i].generation_id),
              i2s(species[i].evolves_from_species_id),
              i2s(species[i].evolution_chain_id),
              i2s(species[i].color_id),
              i2s(species[i].shape_id),
              i2s(species[i].habitat_id),
              i2s(species[i].gender_rate),
              i2s(species[i].capture_rate),
              i2s(species[i].base_happiness),
              i2s(species[i].is_baby),
              i2s(species[i].hatch_counter),
              i2s(species[i].has_gender_differences),
              i2s(species[i].growth_rate_id),
              i2s(species[i].forms_switchable),
              i2s(species[i].is_legendary),
              i2s(species[i].is_mythical),
              i2s(species[i].order),
              i2s(species[i].conquest_order));
    }
    fclose(f);
  }


  prefix = (char *) realloc(prefix, prefix_len + strlen("experience.csv") + 1);
  strcpy(prefix + prefix_len, "experience.csv");
  
  f = fopen(prefix, "r");

  //No null byte copied here, so prefix is not technically a string anymore.
  prefix = (char *) realloc(prefix, prefix_len + 1);

  fgets(line, 800, f);
  
  for (i = 1; i < 601; i++) {
    fgets(line, 800, f);
    experience[i].growth_rate_id = atoi((tmp = next_token(line, ',')));
    tmp = next_token(NULL, ',');
    experience[i].level = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    experience[i].experience =  (*tmp != '\n') ? atoi(tmp) : INT_MAX;
  }

  fclose(f);

  if (print) {
    f = fopen("experience.csv", "w");
    for (i = 1; i < 601; i++) {
      fprintf(f, "%s,%s,%s\n",
              i2s(experience[i].growth_rate_id),
              i2s(experience[i].level),
              i2s(experience[i].experience));
    }
    fclose(f);
  }

  prefix = (char *) realloc(prefix, prefix_len + strlen("type_names.csv") + 1);
  strcpy(prefix + prefix_len, "type_names.csv");
  
  f = fopen(prefix, "r");

  //No null byte copied here, so prefix is not technically a string anymore.
  prefix = (char *) realloc(prefix, prefix_len + 1);

  fgets(line, 800, f);
  
  for (i = 1; i < 19; i++) {
    fgets(line, 800, f); //  1
    fgets(line, 800, f); //  3
    fgets(line, 800, f); //  4
    fgets(line, 800, f); //  5
    fgets(line, 800, f); //  6
    fgets(line, 800, f); //  7
    fgets(line, 800, f); //  8
    fgets(line, 800, f); //  9 - English
    for (j = count = 0; count < 2; j++) {
      if (line[j] == ',') {
        count++;
      }
    }
    line[strlen(line) - 1] = '\0';
    types[i] = strdup(line + j);
    fgets(line, 800, f); // 11
    fgets(line, 800, f); // 12
  }

  fclose(f);

  if (print) {
    f = fopen("type_names.csv", "w");
    for (i = 1; i < 19; i++) {
      fprintf(f, "%s\n", types[i]);
    }
    fclose(f);
  }

  prefix = (char *) realloc(prefix,
                            prefix_len + strlen("pokemon_stats.csv") + 1);
  strcpy(prefix + prefix_len, "pokemon_stats.csv");
  
  f = fopen(prefix, "r");

  //No null byte copied here, so prefix is not technically a string anymore.
  prefix = (char *) realloc(prefix, prefix_len + 1);

  fgets(line, 800, f);
  
  for (i = 1; i < 6553; i++) {
    fgets(line, 800, f);
    pokemon_stats[i].pokemon_id = atoi((tmp = next_token(line, ',')));
    tmp = next_token(NULL, ',');
    pokemon_stats[i].stat_id = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    pokemon_stats[i].base_stat =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    pokemon_stats[i].effort =  (*tmp != '\n') ? atoi(tmp) : INT_MAX;
  }

  fclose(f);

  if (print) {
    f = fopen("pokemon_stats.csv", "w");
    for (i = 1; i < 6553; i++) {
      fprintf(f, "%s,%s,%s,%s\n",
              i2s(pokemon_stats[i].pokemon_id),
              i2s(pokemon_stats[i].stat_id),
              i2s(pokemon_stats[i].base_stat),
              i2s(pokemon_stats[i].effort));
    }
    fclose(f);
  }
  
  prefix = (char *) realloc(prefix, prefix_len + strlen("stats.csv") + 1);
  strcpy(prefix + prefix_len, "stats.csv");
  
  f = fopen(prefix, "r");

  //No null byte copied here, so prefix is not technically a string anymore.
  prefix = (char *) realloc(prefix, prefix_len + 1);

  fgets(line, 800, f);
  
  for (i = 1; i < 9; i++) {
    fgets(line, 800, f);
    stats[i].id = atoi((tmp = next_token(line, ',')));
    tmp = next_token(NULL, ',');
    stats[i].damage_class_id = *tmp ? atoi(tmp) : INT_MAX;
    strcpy(stats[i].identifier, (tmp = next_token(NULL, ',')));
    tmp = next_token(NULL, ',');
    stats[i].is_battle_only =  *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    stats[i].game_index =  (*tmp != '\n') ? atoi(tmp) : INT_MAX;
  }

  fclose(f);
  
  if (print) {
    f = fopen("stats.csv", "w");
    for (i = 1; i < 9; i++) {
      fprintf(f, "%s,%s,%s,%s,%s\n",
              i2s(stats[i].id),
              i2s(stats[i].damage_class_id),
              stats[i].identifier,
              i2s(stats[i].is_battle_only),
              i2s(stats[i].game_index));
    }
    fclose(f);
  }

  prefix = (char *) realloc(prefix,
                            prefix_len + strlen("pokemon_types.csv") + 1);
  strcpy(prefix + prefix_len, "pokemon_types.csv");
  
  f = fopen(prefix, "r");

  //No null byte copied here, so prefix is not technically a string anymore.
  prefix = (char *) realloc(prefix, prefix_len + 1);

  fgets(line, 800, f);
  
  for (i = 1; i < 1676; i++) {
    fgets(line, 800, f);
    pokemon_types[i].pokemon_id = atoi((tmp = next_token(line, ',')));
    tmp = next_token(NULL, ',');
    pokemon_types[i].type_id = *tmp ? atoi(tmp) : INT_MAX;
    tmp = next_token(NULL, ',');
    pokemon_types[i].slot = (*tmp != '\n') ? atoi(tmp) : INT_MAX;
  }

  fclose(f);
  
  if (print) {
    f = fopen("pokemon_types.csv", "w");
    for (i = 1; i < 1676; i++) {
      fprintf(f, "%s,%s,%s\n",
              i2s(pokemon_types[i].pokemon_id),
              i2s(pokemon_types[i].type_id),
              i2s(pokemon_types[i].slot));
    }
    fclose(f);
  }


  free(prefix);
}
