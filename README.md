# soa-contract-first

Two autonomous services in C talking over HTTP with strict JSON contracts.

**Thesis:** Early-2000s SOA (WSDL, UDDI, ESB) was buried under verbose XML and heavy middleware. But the ideas beneath it — contract-first design, autonomous services, bounded contexts — never stopped being relevant. This repo treats SOA not as legacy middleware but as a first-principles reference: services are boundaries, contracts are machine-readable, and integration is explicit.

This is **repo #3** in the compounding series on `github.com/Powerworks`:

```
C message passing  ──►  Ada contracts  ──►  SOA services  ──►  Elixir actors  ──►  Swift/Kotlin mobile  ──►  WASM / eBPF / HTTP3
```

## Services

| Service | Port | Responsibility |
|---------|------|----------------|
| `order-service` | 8081 | Accepts orders, checks inventory via contract |
| `inventory-service` | 8082 | Holds stock, reserves/releases items via contract |
| `registry` | 8080 | Service discovery (name → host:port) |

## What's inside

| Path | Purpose |
|------|---------|
| `src/` | C99 implementation: mini HTTP server, JSON contract emitters, service logic, registry |
| `contracts/` | Machine-readable request/response schemas (JSON) shared between services |
| `design/` | System design narrative, ADRs, Mermaid diagrams |
| `architecture/` | Principal-level architecture extract (Obsidian-friendly) |
| `docs/` | Compounding links to previous/next projects |

## Build

```bash
cmake -S . -B build
cmake --build build
```

Run:

```bash
# Terminal 1
./build/registry 8080

# Terminal 2
./build/inventory-service 8082 127.0.0.1:8080

# Terminal 3
./build/order-service 8081 127.0.0.1:8080 127.0.0.1:8082
```

Then:

```bash
curl http://127.0.0.1:8081/order \
  -X POST -H "Content-Type: application/json" \
  -d '{"item":"widget","quantity":2}'
```

## Design summary

- **Contract first.** `contracts/` defines the exact request/response shape. Services implement to the contract; nothing is implied.
- **Autonomous services.** Each service owns its state and process. They communicate only over HTTP.
- **Explicit boundaries.** No shared memory, no ESB, no hidden coupling. The contract *is* the boundary.
- **Registry-based discovery.** Services register themselves; consumers look them up by name.

See `design/README.md` for the full system design and `architecture/ARCHITECTURE.md` for the principal-level extract.
