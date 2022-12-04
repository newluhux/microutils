#ifndef _TERMCAP_H_
#define _TERMCAP_H_

/*

import from busybox: editors/vi.c

  tiny vi.c: A small 'vi' clone
  Copyright (C) 2000, 2001 Sterling Huxley <sterling@europa.com>
 
 Licensed under GPLv2 or later, see file LICENSE in this source tree.

*/

// VT102 ESC sequences.
// See "Xterm Control Sequences"
// http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
#define ESC "\033"
// Clear-to-end-of-screen.
// (We use default param here.
// Full sequence is "ESC [ <num> J",
// <num> is 0/1/2 = "erase below/above/all".)
#define ESC_CLEAR2EOS          ESC"[J"
// Cursor to given coordinate (1,1: top left)
#define ESC_SET_CURSOR_POS     ESC"[%u;%uH"

#endif
