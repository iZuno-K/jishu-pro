#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h> /*getopt_long()*/

#include <fcntl.h> /*low-level()*/
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/type.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h> /*for videodev2.h>*/

#include <linux/videodev2.h>

//xの先頭からxのサイズ分0をセットする
#define CLEAR(x) memset (&(x), 0, sizeof (x))

//_buffer（）タグ名型構造体をbufferという名前で宣言できるようにする。
typedef struct _buffer {
  void *start;
  size_t length;
} buffer;
