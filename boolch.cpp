// Cilk computation of the number of chain in the boolean lattice
// Should returns n!

#include <atomic>
#include <iostream>
#include <array>
#include <x86intrin.h>
#include <cilk/cilk.h>
#include <cilk/cilk_undocumented.h>

const unsigned int n = 25;
const unsigned int npow = 1 << n;

typedef __uint128_t longuint;
// There is no overload for << for __uint128_t
std::ostream&
operator<<( std::ostream& dest, longuint value )
{
    std::ostream::sentry s( dest );
    if ( s ) {
        longuint tmp = value < 0 ? -value : value;
        char buffer[ 128 ];
        char* d = std::end( buffer );
        do
        {
            -- d;
            *d = "0123456789"[ tmp % 10 ];
            tmp /= 10;
        } while ( tmp != 0 );
        if ( value < 0 ) {
            -- d;
            *d = '-';
        }
        int len = std::end( buffer ) - d;
        if ( dest.rdbuf()->sputn( d, len ) != len ) {
            dest.setstate( std::ios_base::badbit );
        }
    }
    return dest;
}


struct node {
  std::atomic<unsigned char> ready;
  longuint res;
};

std::array< node, npow > nodes;

void do_node(unsigned int v) {
  if (v == 0) {
    nodes[v].res = 1;
  }
  else {
    longuint res = 0;
    for (unsigned int i=0; i < n; i++) {
      unsigned int pred = (v & ~(1 << i));
      if (pred != v) res += nodes[pred].res;
    }
    nodes[v].res = res;
  }
  for (unsigned int i=0; i < n; i++) {
    unsigned int succ = v | (1 << i);
    if (succ != v) {
      if (--nodes[succ].ready == 0) { // pre-decrement is supposed to be atomic !
	cilk_spawn do_node(succ);
	// do_node(succ);
      }
    }
  }
}

int main (void) {
  for (unsigned int i = 0; i < npow; i++)
    nodes[i].ready = _mm_popcnt_u32(i);
  do_node(0);
  // for (unsigned int i = 0; i < npow; i++)
  //  std::cout << i << " " << vertices[i] << " " << results[i] << std::endl;
  std::cout << nodes[npow-1].res << std::endl;
  __cilkrts_dump_stats();
}
