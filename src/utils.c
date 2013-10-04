#include <stdlib.h>

short calcChance(short chance) {
    // notice that here we made a copy of chance, so even though it's volatile it is still ok
    return (chance == 1000) || ((rand() & 0x3FF) < chance);
}