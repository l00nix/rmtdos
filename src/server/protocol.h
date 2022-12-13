// server/protocol.h
//
// Implements packet handling and connection logic.

#include "common/protocol.h"

// Execute startup logic.
extern void protocol_init();

// Process packets from the 'ready queue'.
extern void protocol_process();
