#ifndef DB_PARSE_H
# define DB_PARSE_H

#include <vector>

struct pokemon_db {
  int id;
  char identifier[30];
  int species_id;
  int height;
  int weight;
  int base_experience;
  int order;
  int is_default;
};

struct move_db {
  int id;
  char identifier[50];
  int generation_id;
  int type_id;
  int power;
  int pp;
  int accuracy;
  int priority;
  int target_id;
  int damage_class_id;
  int effect_id;
  int effect_chance;
  int contest_type_id;
  int contest_effect_id;
  int super_contest_effect_id;
};

struct pokemon_move_db {
  int pokemon_id;
  int version_group_id;
  int move_id;
  int pokemon_move_method_id;
  int level;
  int order;
};

struct levelup_move {
  int level;
  int move;
};

struct pokemon_species_db {
  int id;
  char identifier[30];
  int generation_id;
  int evolves_from_species_id;
  int evolution_chain_id;
  int color_id;
  int shape_id;
  int habitat_id;
  int gender_rate;
  int capture_rate;
  int base_happiness;
  int is_baby;
  int hatch_counter;
  int has_gender_differences;
  int growth_rate_id;
  int forms_switchable;
  int is_legendary;
  int is_mythical;
  int order;
  int conquest_order;

  pokemon_species_db() : levelup_moves() {}
  virtual ~pokemon_species_db() {}

  std::vector<levelup_move> levelup_moves;
  std::vector               <int> types;
  int                       base_stat[6];
};

struct experience_db {
  int growth_rate_id;
  int level;
  int experience;
};

struct pokemon_stats_db {
  int pokemon_id;
  int stat_id;
  int base_stat;
  int effort;
};

struct stats_db {
  int id;
  int damage_class_id;
  char identifier[30];
  int is_battle_only;
  int game_index;
};

struct pokemon_types_db {
  int pokemon_id;
  int type_id;
  int slot;
};

extern pokemon_move_db pokemon_moves[528239];
extern pokemon_db pokemon[1093];
extern char *types[19];
extern move_db moves[845];
extern pokemon_species_db species[899];
extern experience_db experience[601];
extern pokemon_stats_db pokemon_stats[6553];
extern stats_db stats[9];
extern pokemon_types_db pokemon_types[1676];

void db_parse(bool print);

#endif
