#ifndef PAIR_H
# define PAIR_H

typedef enum dim {
  dim_x,
  dim_y,
  num_dims
} dim_t;

typedef int16_t pair_t[num_dims];

// class pair_t {
//  public:
//   int16_t pair[num_dims];
//
//   pair_t(int16_t x, int16_t y) {
//     pair[dim_x] = x;
//     pair[dim_y] = y;
//   }
//   pair_t() : pair_t(0, 0) {}
//
//   int16_t& operator[](int i)
//   {
//     return pair[i];
//   }
//   int16_t operator[](int i) const
//   {
//     return pair[i];
//   }
//   void operator=(const pair_t& gets)
//   {
//     pair[dim_x] = gets.pair[dim_x];
//     pair[dim_y] = gets.pair[dim_y];
//   }
// };

// void operator=(pair_t gets, pair_t sets) {
//   gets[dim_x] = sets[dim_x];
//   gets[dim_y] = sets[dim_y];
// }

#endif