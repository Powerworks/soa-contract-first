# Compounding series

This is **repo #3** in a sequence where each project extracts architecture bits and feeds the next.

## Sequence

| # | Repo | Language/Stack | Core idea extracted |
|---|------|----------------|---------------------|
| 1 | [`c-message-passing`](https://github.com/Powerworks/c-message-passing) | C99 | Actor message passing, mailbox isolation |
| 2 | [`ada-contract-state-machine`](https://github.com/Powerworks/ada-contract-state-machine) | Ada 2012 | Design-by-contract, protected objects |
| 3 | `soa-contract-first` | C99 / HTTP/JSON | Service contracts as bounded boundaries |
| 4 | `elixir-supervision-pool` | Elixir/OTP | Supervision trees, let-it-crash |
| 5 | `swift-notes-offline-first` | Swift | Value types, actors, offline-first UI |
| 6 | `kotlin-tasks-multiplatform` | Kotlin | Structured concurrency, repository protocol |
| 7 | `wasm-actor-runtime` | C → WASM | Sandboxed message passing at the bytecode level |
| 8 | `ebpf-observability` | C + eBPF | Programmable kernelspace tracing & networking |
| 9 | `http3-message-gateway` | C / HTTP/3 | QUIC-based message transport, datagram-first semantics |

## What this repo contributes downstream

- **From C message passing:** Mailbox isolation → service process boundary; messages become HTTP requests.
- **From Ada contracts:** `Pre`/`Post` → JSON request/response schemas; contract is executable, not documentation.
- **To Elixir:** Service boundaries → OTP processes; HTTP failure → supervision restart strategies.
- **To Swift/Kotlin:** Mobile clients consume SOA contracts; repository protocol mirrors the service interface.

## Obsidian links

[[Programming/C|c-message-passing]]  
[[Programming/Ada|ada-contract-state-machine]]  
[[Programming/SOA|soa-contract-first]]  
[[Programming/Elixir|elixir-supervision-pool (planned)]]
