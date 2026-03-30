---
name: add-network-message
description: Add a new client-to-server or server-to-client network message. Use when a new game event, state update, or RPC needs to be communicated over the network.
argument-hint: "<MessageName> [client-to-server|server-to-client|both]"
context: fork
agent: Plan
---

# Add New Network Message: $ARGUMENTS

Add a new network message to the InterstellarOutpost MMO networking layer.

## Steps

1. **Understand existing messages** — Read `NeuronClient/clienttoserver.h` / `clienttoserver.cpp` and `NeuronServer/servertoclient.h` / `servertoclient.cpp`. Read `NeuronCore/net_lib.h`, `net_udp_packet.h`, and `binary_stream_readers.h` to understand the serialisation primitives. Review `NeuronCore/bandwidth.h` for bandwidth tracking patterns.

2. **Define the message struct/type**:
   - Add a new message type constant (enum or `#define`) in the appropriate header
   - Define a struct carrying the message payload — keep it compact; prefer fixed-width integer types
   - Consider delta-compression or dirty flags if the message is sent frequently

3. **Serialise (write side)**:
   - In the appropriate `clienttoserver.cpp` or `servertoclient.cpp`, implement the function that packs the struct into the outgoing `net_udp_packet` / binary stream
   - Register any bandwidth accounting via `NeuronCore/bandwidth.h`

4. **Deserialise (read side)**:
   - In the receiving end (`NeuronServer/server.cpp` or `NeuronClient/NeuronClient.cpp`), add a case to the message-dispatch switch/if chain
   - Implement the handler that unpacks the stream and applies the game-state change

5. **Hook into the update loop**:
   - Send side: call the new send function from the relevant game tick (e.g., `InterstellarOutpost/location.cpp` or `InterstellarOutpost/multiwinia.cpp`)
   - Receive side: the existing polling loop in `server.cpp` / `NeuronClient.cpp` should pick it up automatically once the case is added

6. **Sync/anti-cheat considerations**:
   - Server should validate all values before applying (clamp positions, verify entity ownership)
   - For authoritative state, prefer server-to-client confirmations rather than trusting client values

## Conventions
- Message IDs must not collide with existing ones — grep for existing constants before assigning a new value
- Keep messages small; avoid sending strings unless unavoidable (use IDs/enums)
- Use `NeuronCore/binary_stream_readers.h` read/write helpers consistently
