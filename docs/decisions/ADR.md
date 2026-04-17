# Architecture Decision Records

This directory tracks every major technical decision made across the Kirov Dynamics ecosystem.
Use the ADR format below whenever making a significant architectural, tooling, or infrastructure choice.

---

## ADR-001: Hybrid Modular Monolith over Microservices

**Date:** 2025-Q4
**Status:** Accepted

**Decision:** Adopt a hybrid modular monolith for initial system phases rather than full microservices.

**Reason:**

- Faster iteration speed at early stage scale
- Reduces operational overhead (no inter-service networking, service discovery, etc.)
- Modules are bounded contexts — can be extracted into microservices in Phase 3 with minimal refactoring

---

## ADR-002: PostgreSQL as the Core Data Layer

**Date:** 2025-Q4
**Status:** Accepted

**Decision:** Use PostgreSQL as the primary database for all trust-critical systems (Sumbandila, SupportHive-C).

**Reason:**

- Strong ACID compliance required for financial and identity trust
- First-class support for JSON, geospatial, and time-series queries via extensions
- Supabase provides rapid managed setup for MVP phases

---

## ADR-003: Event-Driven Architecture for Sumbandila

**Date:** 2026-Q1
**Status:** Accepted

**Decision:** Implement event-driven communication between Sumbandila's service modules.

**Reason:**

- Supports real-time processing for trust and identity flows
- Loose coupling enables independent scaling per service domain
- Aligns with the decentralised trust model (no single point of authority)

---

## ADR-004: Zero Trust Security Model

**Date:** 2026-Q1
**Status:** Accepted

**Decision:** Apply Zero Trust principles across all system boundaries.

**Rules:**

- No implicit trust, even between internal services
- All service-to-service calls require authentication tokens
- Audit logs for all privileged operations
- Principle of least privilege enforced at the infrastructure layer

---

## ADR-005: Docker-first Local Development

**Date:** 2026-Q1
**Status:** Accepted

**Decision:** All services must be runnable locally via `docker-compose up` with no manual environment setup.

**Reason:**

- Eliminates "works on my machine" inconsistencies
- Forces infrastructure-as-code discipline early
- Paves the way for Kubernetes migration in Phase 3
