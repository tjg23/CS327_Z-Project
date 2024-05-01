#ifndef POKEMON_H
# define POKEMON_H

class move_db;

enum pokemon_stat {
  stat_hp,
  stat_atk,
  stat_def,
  stat_spatk,
  stat_spdef,
  stat_speed
};

enum pokemon_gender {
  gender_female,
  gender_male
};

class pokemon {
 private:
//  public:
  int level;
  int hp;
  int pokemon_index;
  int type[2];
  int move_index[4];
  int pokemon_species_index;
  int IV[6];
  int effective_stat[6];
  bool shiny;
  pokemon_gender gender;
 public:
  pokemon();
  pokemon(int level);
  int get_lvl() const;
  const char *get_species() const;
  int get_hp() const;
  int get_chp() const;
  int get_atk() const;
  int get_def() const;
  int get_spatk() const;
  int get_spdef() const;
  int get_speed() const;
  const char *get_gender_string() const;
  bool is_shiny() const;
  const char *get_move(int i) const;
  // move_db* get_move_data(int i);
  int move_priority(int i);
  int move_accuracy(int i);

  int hit(int dmg);
  int heal(int p);
  int attack(int move_id, pokemon& target);
};

#endif
