#include <ref.h>

typedef struct socket_t socket_t;

struct socket_t {
  ref_obj_t ref;
  page_t buffer[3];
} __attribute__((packed));

