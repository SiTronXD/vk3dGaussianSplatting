
// TODO: move these to specialization constants eventually

// Has to reflect RS_BITS_PER_PASS in Renderer.h
#define BITS_PER_PASS_SIZE 4u
#define BIN_COUNT (1u << BITS_PER_PASS_SIZE)
#define SHIFT_MASK (BIN_COUNT - 1u)