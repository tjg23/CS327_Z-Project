#ifndef PAIR_H
# define PAIR_H

typedef enum dim {
  dim_x,
  dim_y,
  num_dims
} dim_t;

typedef int16_t pair_t[num_dims];

/* ###### Pair Operations */
	#define pairCpy(dest, src) ({   \
  	dest[dim_x] = src[dim_x];     \
  	dest[dim_y] = src[dim_y];     \
	})
	#define pairSet(dest, x, y) ({  \
  	dest[dim_x] = x;              \
  	dest[dim_y] = y;              \
	})
	#define pairDiff(diff, key, with) ({      \
  	diff[dim_x] = key[dim_x] - with[dim_x]; \
  	diff[dim_y] = key[dim_y] - with[dim_y]; \
	})

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
