# ADR 0001 — Contract-first SOA

## Status

Accepted

## Context

Early-2000s SOA paired contract-first design with WSDL, UDDI, and heavyweight ESBs. The XML verbosity and centralised bus killed the style for many teams, who moved to "just use REST/JSON and figure it out." That swung too far: no contracts, implicit coupling, distributed monoliths.

We considered:
1. **No contract, implicit REST** — fast to start, impossible to evolve safely.
2. **ESB / message broker** — adds operational complexity and a single point of coupling.
3. **Contract-first, autonomous services, HTTP + JSON** — explicit boundaries, machine-readable schemas, no central bus.

## Decision

Use contract-first autonomous services over plain HTTP/JSON. Each service owns its state and process. Communication is via explicit request/response matching the schemas in `contracts/`. No ESB, no shared database.

## Consequences

### Positive

- **Boundaries are explicit.** Crossing a service boundary means an HTTP call with a defined contract — no hidden coupling.
- **Independent deployment.** Each service builds and runs alone.
- **Machine-readable contracts.** `contracts/*.json` can drive client code generation, mock servers, and contract tests.

### Negative

- **Network failure modes.** Calls can fail, timeout, or partially succeed. The order service must handle inventory being down.
- **Latency.** HTTP round-trips are slower than in-process calls.
- **No transactions across services.** Reserve/check is not atomic; sagas or compensating actions are needed for production.

## Related

- [[../architecture/ARCHITECTURE]] — principal extract
- [[../../ada-contract-state-machine/architecture/ARCHITECTURE|Ada contracts]] — pre/post conditions as executable spec; SOA extends that idea across process boundaries
- [[../../c-message-passing/architecture/ARCHITECTURE|C message passing]] — mailbox isolation; HTTP is the network mailbox
