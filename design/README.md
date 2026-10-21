# System design — SOA contract-first

## Scope

Three C99 processes: `registry`, `inventory-service`, `order-service`. Each is an autonomous service with its own process, its own state, and its own HTTP boundary. They communicate only via HTTP/1.1 with JSON bodies shaped by the shared contracts in `contracts/`.

## Components

### Registry (`src/registry.c`)

In-memory name → address map.
- `POST /register` — service registers `{"name": "...", "address": "host:port"}`
- `GET /lookup?name=X` — returns `{"name": "...", "address": "..."}`

Services register on startup. Consumers look up before calling. In this demo the order-service has the inventory address hardcoded for clarity; the registry demonstrates the discovery pattern.

### Inventory service (`src/inventory_service.c`)

Owns stock state (`widget`, `gadget`).
- `POST /inventory/check` — `{"item","quantity"}` → `{"available": bool}`
- `POST /inventory/reserve` — `{"item","quantity","order_id"}` → `{"reserved": bool, "order_id": ...}`

State is private to the process. No other service can touch it directly.

### Order service (`src/order_service.c`)

Owns order intake. On `POST /order`:
1. Calls inventory `/inventory/check` over HTTP
2. If available, calls inventory `/inventory/reserve`
3. Returns `{"order_id","status","item","quantity"}`

The order service does not know how inventory stores stock — only its contract.

## Contract model

`contracts/*.json` are the machine-readable source of truth for request/response shapes. The C code implements to them; the contracts do not describe the code, the code describes the contracts.

## Decisions recorded

See `design/adr/0001-contract-first.md`.

## Diagrams

See `design/diagrams/service-topology.mmd`.
