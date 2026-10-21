# Architecture extract — soa-contract-first

**Level:** Principal  
**Purpose:** Extract architecture-relevant bits that compound into the next projects in this series.  
**Format:** Obsidian-friendly markdown with `[[wikilinks]]`.

## Signal (carry forward)

| Pattern | Observation | Why it matters |
|---------|-------------|----------------|
| **Contract as boundary** | `contracts/*.json` defines request/response; services implement to it. | The contract is not documentation; it is the interface. This is Ada's `Pre`/`Post` lifted across process boundaries. |
| **Autonomous services** | Each service owns state and process; no shared DB, no ESB. | Failure in one service cannot corrupt another's state. Isolation by process boundary. |
| **Explicit network calls** | Order service calls inventory over HTTP; no hidden coupling. | Every integration is visible and testable. |
| **Discovery via registry** | Name → address lookup decouples caller from concrete location. | Location transparency without a central bus. |

## Noise (deliberately skipped at this layer)

- **ESB / message broker.** Centralised routing reintroduces the coupling SOA was meant to remove.
- **Distributed transactions.** 2PC across services is an anti-pattern at this scale; compensating actions/sagas are the better model.
- **Service mesh / sidecars.** Operational complexity not needed for a reference implementation.

## Principal takeaway

> A service boundary is a contract, not a technology choice. HTTP/JSON happens to be the wire format today; the architecture is the explicit, machine-readable contract on each side of the boundary.

Early SOA got the ideas right but wrapped them in XML and ESBs that made them expensive. We're returning to the same first principles with cheaper tooling.

## Compounds into

- [[../../ada-contract-state-machine/architecture/ARCHITECTURE|Ada contracts]] — `Pre`/`Post` on transitions is the same idea as service request/response schemas
- [[../../c-message-passing/architecture/ARCHITECTURE|C message passing]] — mailboxes are local service boundaries; HTTP is the network mailbox
- [[../../../elixir-supervision-pool/docs/COMPOUNDING|Elixir supervision (planned)]] — service failure → supervisor restart; SOA teaches us where boundaries fail
- [[../../../swift-notes-offline-first/docs/COMPOUNDING|Swift/Kotlin (planned)]] — mobile clients consume SOA contracts; repository protocol mirrors service interface
