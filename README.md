# DNS Zone Manager

[![Build](https://github.com/kuchta-martin/dns-zone-manager/actions/workflows/build.yml/badge.svg)](https://github.com/kuchta-martin/dns-zone-manager/actions/workflows/build.yml)

A C++20 library for managing hierarchical DNS zones with multiple record types, conflict detection, and dot-path search.

Developed as a semester project for [PA2 – Programming and Algorithmics 2](https://fit.cvut.cz/) at FIT CTU Prague.  
**Progtest score: 7.70 / 7.00** (above-maximum bonus points awarded).

---

## The Problem

A DNS zone is a tree of named records. Each record has a type (A, AAAA, MX, CNAME, SPF) and associated data. Zones can be nested to arbitrary depth, enabling hierarchical lookups like `progtest.fit.cvut.cz`.

The challenge: enforce RFC-compliant conflict rules (CNAME exclusivity, zone-name uniqueness), support deep copy semantics, and allow searches that return *references* into the live zone tree — not copies.

---

## Design

```
include/
  rec_base.hpp      – CRec: abstract base, virtual clone() → unique_ptr
  rec_types.hpp     – CRecA, CRecAAAA, CRecMX, CRecCNAME, CRecSPF
  search_result.hpp – CSearchResult: non-owning view into a zone
  zone.hpp          – CZone: owning composite (unique_ptr storage)
src/
  zone.cpp          – CZone implementation
  main.cpp          – smoke tests + demo
```

### Key design decisions

| Decision | Rationale |
|---|---|
| `std::vector<std::unique_ptr<CRec>>` in CZone | Eliminates manual `delete`; destructor and copy-assignment are clean |
| `virtual std::unique_ptr<CRec> clone()` | Type-safe deep copy without raw heap ownership transfer |
| `CSearchResult` holds non-owning raw pointers | Zero-copy access to live records; caller is responsible for zone lifetime |
| Composite pattern (CZone is-a CRec) | Zones nest to arbitrary depth with no special-casing at the call site |
| CNAME conflict via `conflictsWith()` | Polymorphic dispatch — new record types can define their own rules |

### Conflict rules enforced by `CZone::add()`

- **Duplicates** — exact match via `equals()` → rejected
- **CNAME exclusivity** — a CNAME blocks all other records with the same name (and vice versa)
- **Zone name uniqueness** — two sub-zones with the same name cannot coexist

---

## Build

Requires a C++20 compiler and CMake ≥ 3.16.

```bash
cmake -S . -B build
cmake --build build
./build/dns-manager
```

Compiled with `-Wall -Wextra -Wpedantic -Wconversion -Wshadow` — **zero warnings**.

### Manual (no CMake)

```bash
clang++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Iinclude \
        src/main.cpp src/zone.cpp -o dns-manager
```

---

## Usage

```cpp
CZone root("<ROOT ZONE>");
CZone cz("cz");
CZone fit("fit");

fit.add(CRecA    ("www",  CIPv4("147.32.232.142")));
fit.add(CRecAAAA ("www",  CIPv6("2001:718:2:2902:0:1:2:3")));
fit.add(CRecMX   ("fit",  "mail.fit.cvut.cz.", 10));
fit.add(CRecCNAME("alias","www.fit.cvut.cz."));
fit.add(CRecSPF  ("fit").add("ip4:147.32.232.0/24"));

cz.add(fit);
root.add(cz);

// Tree output
std::cout << root;

// Dot-path search — returns non-owning references into the live tree
CSearchResult hits = root.search("www.fit.cz");
std::cout << hits;                         // prints all matching records
dynamic_cast<CRecA&>(hits[0]);             // safe downcast

// Conflict detection
fit.add(CRecCNAME("www", "other.cz."));   // returns false — "www" has A records
```

### Expected output

```
<ROOT ZONE>
 \- cz
    \- fit
       +- www A 147.32.232.142
       +- www AAAA 2001:718:2:2902:0:1:2:3
       +- fit MX 10 mail.fit.cvut.cz.
       +- alias CNAME www.fit.cvut.cz.
       \- fit SPF ip4:147.32.232.0/24

www A 147.32.232.142
www AAAA 2001:718:2:2902:0:1:2:3
```

---

## Record types

| Type | Class | Data |
|---|---|---|
| A | `CRecA` | IPv4 address (`CIPv4`) |
| AAAA | `CRecAAAA` | IPv6 address (`CIPv6`) |
| MX | `CRecMX` | priority + mail hostname |
| CNAME | `CRecCNAME` | canonical name (alias) |
| SPF | `CRecSPF` | list of IP range specifiers (fluent `.add()`) |
| Zone | `CZone` | named container for records and sub-zones |

---

## Author

Martin Kuchta — FIT CTU Prague, 1st year  
[github.com/kuchta-martin](https://github.com/kuchta-martin)
