#pragma once

#include "rec_base.hpp"

// Suppress warnings from the FIT-provided header (not our code).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#include "ipaddress.h"
#pragma clang diagnostic pop

#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

// ── A record (IPv4 address) ──────────────────────────────────────────────────

class CRecA : public CRec {
    CIPv4 m_address;
public:
    CRecA(const std::string& name, const CIPv4& address)
        : CRec(name, "A"), m_address(address) {}

    void printAddr(std::ostream& os) const override {
        os << m_name << " " << m_type << " " << m_address;
    }
    bool equals(const CRec& other) const override {
        const auto* o = dynamic_cast<const CRecA*>(&other);
        return o && m_name == o->m_name && m_address == o->m_address;
    }
    std::unique_ptr<CRec> clone() const override {
        return std::make_unique<CRecA>(*this);
    }
};

// ── AAAA record (IPv6 address) ───────────────────────────────────────────────

class CRecAAAA : public CRec {
    CIPv6 m_address;
public:
    CRecAAAA(const std::string& name, const CIPv6& address)
        : CRec(name, "AAAA"), m_address(address) {}

    void printAddr(std::ostream& os) const override {
        os << m_name << " " << m_type << " " << m_address;
    }
    bool equals(const CRec& other) const override {
        const auto* o = dynamic_cast<const CRecAAAA*>(&other);
        return o && m_name == o->m_name && m_address == o->m_address;
    }
    std::unique_ptr<CRec> clone() const override {
        return std::make_unique<CRecAAAA>(*this);
    }
};

// ── MX record (mail exchange) ────────────────────────────────────────────────

class CRecMX : public CRec {
    std::string m_address;
    int         m_priority;
public:
    CRecMX(std::string name, std::string address, int priority)
        : CRec(std::move(name), "MX")
        , m_address(std::move(address))
        , m_priority(priority) {}

    void printAddr(std::ostream& os) const override {
        os << m_name << " " << m_type << " " << m_priority << " " << m_address;
    }
    bool equals(const CRec& other) const override {
        const auto* o = dynamic_cast<const CRecMX*>(&other);
        return o && m_name == o->m_name
                 && m_address == o->m_address
                 && m_priority == o->m_priority;
    }
    std::unique_ptr<CRec> clone() const override {
        return std::make_unique<CRecMX>(*this);
    }
};

// ── CNAME record (canonical name / alias) ────────────────────────────────────

class CRecCNAME : public CRec {
    std::string m_address;
public:
    CRecCNAME(std::string name, std::string address)
        : CRec(std::move(name), "CNAME"), m_address(std::move(address)) {}

    void printAddr(std::ostream& os) const override {
        os << m_name << " " << m_type << " " << m_address;
    }
    bool equals(const CRec& other) const override {
        const auto* o = dynamic_cast<const CRecCNAME*>(&other);
        return o && m_name == o->m_name && m_address == o->m_address;
    }
    std::unique_ptr<CRec> clone() const override {
        return std::make_unique<CRecCNAME>(*this);
    }
    /// A CNAME must be the only record for its name (RFC 1034 §3.6.2).
    bool conflictsWith(const CRec& other) const override {
        return name() == other.name();
    }
};

// ── SPF record (sender policy framework) ─────────────────────────────────────

class CRecSPF : public CRec {
    std::vector<std::string> m_addresses;
public:
    explicit CRecSPF(const std::string& name) : CRec(name, "SPF") {}

    /// Fluent builder — append one IP range specifier (e.g. "ip4:10.0.0.0/8").
    CRecSPF& add(std::string addr) {
        m_addresses.push_back(std::move(addr));
        return *this;
    }

    void printAddr(std::ostream& os) const override {
        os << m_name << " " << m_type << " ";
        for (std::size_t i = 0; i < m_addresses.size(); ++i) {
            if (i > 0) os << ", ";
            os << m_addresses[i];
        }
    }
    bool equals(const CRec& other) const override {
        const auto* o = dynamic_cast<const CRecSPF*>(&other);
        return o && m_name == o->m_name && m_addresses == o->m_addresses;
    }
    std::unique_ptr<CRec> clone() const override {
        return std::make_unique<CRecSPF>(*this);
    }
};
