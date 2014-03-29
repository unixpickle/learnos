typedef struct {
  uint64_t bus;
  uint64_t device;
  uint64_t function;
  uint64_t reg; // 4-byte aligned, so first two bits = 0
} __attribute__((packed)) pci_address;

