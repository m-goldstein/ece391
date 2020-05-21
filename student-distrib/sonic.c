#ifndef SONIC_C
#define SONIC_C
#include "include/lib.h"
#include "include/sonic.h"
static char sonic[] ="          ______                         		    \n\
                    _.-*'-\"/\"'      '\"\\-``*-.                           \n\
                 .-'                            `-.                         \n\
      /`-.    .-'                  _.              `-.                      \n\
     :    `..'                  .-'_ .                `.                    \n\
     |    .'                 .-'_.'   ).                )                   \n\
     |   /                 .' .*     ;               .-/                    \n\
     :   L                    `.     | ;          .-'                       \n\
      \\.' `*.         .-*\"*-.  `.   ; |        .'                         \n\
      /      )        '       `.  `-'  ;      .'                            \n\
     : .'\"`.  .       .-*'`*-.  \\    .      (_                            \n\
     |/     |       .'        \\  .             `*-.                        \n\
     |.     .      /           ;                   `-.                      \n\
     |    db      '       d$b  |                      `-.                   \n\
     |   :PT;.   '       :PT;: |                         `.                 \n\
     :   :b_d;   '       :b_d;:|                           |                \n\
     |   :$$; `'         :$$$; |                            )               \n\
     |    TP              T$P  '                            \\              \n\
     :                        /.- *'~.               _ ~  |                 \n\
    .sdP^T$bs.               /'      \\      _/~ / _/ . /                   \n\
    $$$._.$$$$b.--._      _.'   .--.   ;  _/_   . ]                         \n\
    `*$$$$$$P*'     `*--*'     '  / \\ :~/                                  \n\
       \\                       .'   ; ;                                    \n\
        `.                  _.-'    ' /                                     \n\
          `*-.                      .'                                      \n\
              `*-._            _.-*'                                        \n\
                   `*=--..--=*'                                             \n";
/* 
 * splash_sonic
 *  DESCRIPTION: displays an ASCII art of Sonic the Hedgehog
 *  INPUTS: none
 *  OUTPUTS: prints sonic the hedgehog
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
 */
void splash_sonic()
{
    printf("%s", sonic);
}
#endif
