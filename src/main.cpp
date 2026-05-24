#include "zone.hpp"
#include "rec_types.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>

static void smokeTests() {
    std::ostringstream oss;

    // ── Basic zone with multiple record types ────────────────────────────────
    CZone z0("fit");
    assert(z0.add(CRecA    ("progtest", CIPv4("147.32.232.142")))            == true);
    assert(z0.add(CRecAAAA ("progtest", CIPv6("2001:718:2:2902:0:1:2:3")))   == true);
    assert(z0.add(CRecA    ("courses",  CIPv4("147.32.232.158")))            == true);
    assert(z0.add(CRecA    ("courses",  CIPv4("147.32.232.160")))            == true);
    assert(z0.add(CRecA    ("courses",  CIPv4("147.32.232.159")))            == true);
    assert(z0.add(CRecCNAME("pririz",   "sto.fit.cvut.cz."))                == true);
    assert(z0.add(CRecSPF  ("courses").add("ip4:147.32.232.128/25")
                                      .add("ip4:147.32.232.64/26"))          == true);
    assert(z0.add(CRecAAAA ("progtest", CIPv6("2001:718:2:2902:1:2:3:4")))   == true);
    assert(z0.add(CRecMX   ("courses",  "relay.fit.cvut.cz.",   0))          == true);
    assert(z0.add(CRecMX   ("courses",  "relay2.fit.cvut.cz.", 10))          == true);

    oss.str("");
    oss << z0;
    assert(oss.str() ==
        "fit\n"
        " +- progtest A 147.32.232.142\n"
        " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
        " +- courses A 147.32.232.158\n"
        " +- courses A 147.32.232.160\n"
        " +- courses A 147.32.232.159\n"
        " +- pririz CNAME sto.fit.cvut.cz.\n"
        " +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
        " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
        " +- courses MX 0 relay.fit.cvut.cz.\n"
        " \\- courses MX 10 relay2.fit.cvut.cz.\n");

    assert(z0.search("progtest").size() == 3);

    // ── Delete / re-add ──────────────────────────────────────────────────────
    assert(z0.del(CRecA("courses", CIPv4("147.32.232.160"))) == true);
    assert(z0.add(CRecA("courses", CIPv4("147.32.232.122"))) == true);
    assert(z0.search("courses").size() == 6);

    // ── CNAME conflict rules ─────────────────────────────────────────────────
    CZone z10("fit");
    assert(z10.add(CRecA    ("progtest", CIPv4("147.32.232.142")))          == true);
    assert(z10.add(CRecA    ("progtest", CIPv4("147.32.232.142")))          == false); // duplicate
    assert(z10.add(CRecCNAME("pririz",   "sto.fit.cvut.cz."))              == true);
    assert(z10.add(CRecCNAME("pririz",   "stojedna.fit.cvut.cz."))         == false); // CNAME conflict
    assert(z10.add(CRecA    ("pririz",   CIPv4("147.32.232.111")))          == false); // CNAME blocks A
    assert(z10.add(CRecCNAME("progtest", "progtestbak.fit.cvut.cz."))      == false); // A blocks CNAME
    assert(z10.add(CZone    ("test"))                                        == true);
    assert(z10.add(CZone    ("pririz"))                                      == false); // name conflict

    // ── Nested zones and dot-path search ─────────────────────────────────────
    CZone z20("<ROOT ZONE>");
    CZone z21("cz");
    CZone z22("cvut");
    CZone z23("fit");
    assert(z23.add(CRecA   ("progtest", CIPv4("147.32.232.142")))          == true);
    assert(z23.add(CRecAAAA("progtest", CIPv6("2001:718:2:2902:0:1:2:3"))) == true);
    CZone z24("fel");
    assert(z24.add(CRecA   ("www", CIPv4("147.32.80.2")))                  == true);
    assert(z24.add(CRecAAAA("www", CIPv6("1:2:3:4:5:6:7:8")))             == true);
    assert(z22.add(z23) == true);
    assert(z22.add(z24) == true);
    assert(z21.add(z22) == true);
    assert(z20.add(z21) == true);

    assert(z20.search("progtest.fit.cvut.cz").size() == 2);

    // ── out_of_range on invalid index ─────────────────────────────────────────
    try {
        z0.search("courses")[99];
        assert(false && "expected out_of_range");
    } catch (const std::out_of_range&) {}

    std::cout << "All smoke tests passed.\n";
}

int main() {
    smokeTests();

    // ── Demo output ──────────────────────────────────────────────────────────
    CZone root("<ROOT ZONE>");
    CZone cz("cz");
    CZone cvut("cvut");
    CZone fit("fit");

    fit.add(CRecA    ("www",      CIPv4("147.32.232.142")));
    fit.add(CRecAAAA ("www",      CIPv6("2001:718:2:2902:0:1:2:3")));
    fit.add(CRecMX   ("fit",      "mail.fit.cvut.cz.", 10));
    fit.add(CRecCNAME("courses",  "www.fit.cvut.cz."));
    fit.add(CRecSPF  ("fit").add("ip4:147.32.232.0/24").add("ip6:2001:718:2::/48"));

    cvut.add(fit);
    cz.add(cvut);
    root.add(cz);

    std::cout << "\nZone tree:\n" << root;
    std::cout << "\nSearch 'www.fit.cvut.cz':\n"
              << root.search("www.fit.cvut.cz");

    return EXIT_SUCCESS;
}
