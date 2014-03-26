enum {
  MSG_TYPE_DRAIN,
  MSG_TYPE_SERVICE_INIT,
  MSG_TYPE_CLIENT_INIT,
  MSG_TYPE_LOOKUP,
  MSG_TYPE_VERIFY,
  MSG_TYPE_EXISTS,
  MSG_TYPE_NOT_EXISTS,
  MSG_TYPE_FOUND,
  MSG_TYPE_NOT_FOUND
};

typedef struct {
  uint64_t type; // MSG_TYPE_FOUND
  uint64_t serviceId;
  uint64_t pid;
} __attribute__((packed)) msgd_result_t;

