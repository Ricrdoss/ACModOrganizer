#include "BrandDetector.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <set>
#include <unordered_set>

namespace acbo {

namespace {

std::string toLower(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

std::string normalizeText(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == '_' || c == '-' || c == '.') {
            out.push_back(' ');
        } else if (std::isalnum(c)) {
            // Insert space between transition from letter to digit (e.g. abarth500 -> abarth 500)
            if (!out.empty() && std::isalpha(static_cast<unsigned char>(out.back())) && std::isdigit(c)) {
                out.push_back(' ');
            }
            // Insert space between transition from digit to letter (e.g. 500s1 -> 500 s1)
            else if (!out.empty() && std::isdigit(static_cast<unsigned char>(out.back())) && std::isalpha(c)) {
                out.push_back(' ');
            }
            out.push_back(static_cast<char>(std::tolower(c)));
        } else {
            out.push_back(' ');
        }
    }
    return out;
}

std::vector<std::string> splitWords(std::string_view s) {
    std::vector<std::string> words;
    std::string current;
    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            current.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else if (!current.empty()) {
            words.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) {
        words.push_back(current);
    }
    return words;
}

std::string normalizeCountryName(std::string_view c) {
    std::string lower = toLower(c);
    std::string clean;
    for (char ch : lower) {
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == ' ') {
            clean.push_back(ch);
        }
    }
    while (!clean.empty() && clean.front() == ' ') clean.erase(clean.begin());
    while (!clean.empty() && clean.back() == ' ') clean.pop_back();

    if (clean == "usa" || clean == "us" || clean == "eeuu" || clean == "u s a" || clean == "u s" || clean == "united states" || clean == "united states of america" || clean == "america") {
        return "united states";
    }
    if (clean == "uk" || clean == "u k" || clean == "gb" || clean == "gbr" || clean == "great britain" || clean == "britain" || clean == "england" || clean == "united kingdom") {
        return "united kingdom";
    }
    if (clean == "de" || clean == "deu" || clean == "ger" || clean == "germany" || clean == "deutschland") {
        return "germany";
    }
    if (clean == "it" || clean == "ita" || clean == "italy" || clean == "italia") {
        return "italy";
    }
    if (clean == "jp" || clean == "jpn" || clean == "japan" || clean == "nippon") {
        return "japan";
    }
    if (clean == "fr" || clean == "fra" || clean == "france") {
        return "france";
    }
    if (clean == "se" || clean == "swe" || clean == "sweden" || clean == "sverige") {
        return "sweden";
    }
    if (clean == "es" || clean == "esp" || clean == "spain" || clean == "espana") {
        return "spain";
    }
    if (clean == "at" || clean == "aut" || clean == "austria" || clean == "osterreich") {
        return "austria";
    }
    if (clean == "au" || clean == "aus" || clean == "australia") {
        return "australia";
    }
    if (clean == "nl" || clean == "nld" || clean == "netherlands" || clean == "holland") {
        return "netherlands";
    }
    if (clean == "cz" || clean == "cze" || clean == "czech" || clean == "czech republic" || clean == "czechia") {
        return "czech republic";
    }
    if (clean == "kr" || clean == "kor" || clean == "korea" || clean == "south korea") {
        return "south korea";
    }
    if (clean == "dk" || clean == "dnk" || clean == "denmark" || clean == "danmark") {
        return "denmark";
    }
    if (clean == "ro" || clean == "rou" || clean == "romania") {
        return "romania";
    }
    if (clean == "ru" || clean == "rus" || clean == "russia") {
        return "russia";
    }
    if (clean == "tr" || clean == "tur" || clean == "turkey" || clean == "turkiye") {
        return "turkey";
    }
    if (clean == "ae" || clean == "are" || clean == "uae" || clean == "united arab emirates" || clean == "dubai") {
        return "united arab emirates";
    }
    return clean;
}

bool isSameCountry(std::string_view c1, std::string_view c2) {
    if (c1.empty() || c2.empty()) return false;
    return normalizeCountryName(c1) == normalizeCountryName(c2);
}

// Common automotive descriptors and modding slang that must NEVER be fuzzy matched to brand names
const std::unordered_set<std::string> s_ignoredFuzzyWords = {
    "drag", "drift", "turbo", "boost", "super", "street", "track", "race", "speed",
    "power", "beater", "missile", "server", "exclusive", "edition", "spec", "pack",
    "stage", "mod", "tune", "king", "awd", "rwd", "fwd", "v6", "v8", "v10", "v12",
    "twin", "quad", "custom", "sound", "acfl", "acdfr", "actk", "mkelite", "sim",
    "auto", "racing", "motor", "motorsport", "fast", "slow", "wheel", "shift",
    "sport", "sports", "series", "version", "final", "clean", "dirty", "works",
    "garage", "team", "club", "force", "india", "japan", "germany", "italy", "france"
};

} // namespace

BrandDetector::BrandDetector() {
    initDatabase();
    loadBrandBadgesFromSystem();
}

void BrandDetector::addBrand(std::string canonicalBrand, std::string country,
                              std::vector<std::string> brandAliases,
                              std::vector<std::string> modelSynonyms) {
    BrandInfo info{canonicalBrand, country};
    
    // Canonical brand name always goes to primary lookup
    m_brandLookup[toLower(canonicalBrand)] = info;
    m_sortedBrandRules.emplace_back(toLower(canonicalBrand), info);

    // Brand aliases (alternative manufacturer names) go to primary lookup
    for (const auto& alias : brandAliases) {
        m_brandLookup[toLower(alias)] = info;
        m_sortedBrandRules.emplace_back(toLower(alias), info);
    }

    // Model synonyms (model names that imply a brand) go to fallback lookup
    for (const auto& model : modelSynonyms) {
        m_modelSynonyms[toLower(model)] = info;
        m_sortedModelRules.emplace_back(toLower(model), info);
    }

    m_allBrandsSorted.push_back(canonicalBrand);
}

void BrandDetector::initDatabase() {
    // addBrand(canonical, country, {brand_aliases}, {model_synonyms})
    // brand_aliases: Alternative names for the manufacturer itself (PRIMARY matching)
    // model_synonyms: Car model names that imply the brand (FALLBACK matching)

    // -------------------------------------------------------------
    // Germany
    // -------------------------------------------------------------
    addBrand("Porsche", "Germany",
        {"porsche ag", "porsche911", "porsche992"},
        {"911", "991", "992", "996", "997", "964", "993", "gt3", "gt2", "gt3 rs", "gt2 rs",
         "cayman", "boxster", "718", "918", "taycan", "panamera", "carrera"});
    addBrand("BMW", "Germany",
        {"bimmer", "beemer", "bmw m", "bmwm3", "bmwm4", "bmwm5"},
        {"m3", "m4", "m5", "m2", "m8", "m6", "e30", "e36", "e46", "e92", "e90",
         "f80", "f82", "g80", "g82", "e39", "e60", "e28", "e34", "e38", "e31", "e24", "z3", "z4"});
    addBrand("Mercedes-Benz", "Germany",
        {"mercedes", "merc", "mercedes amg", "mercedes-amg", "mercedez"},
        {"amg", "c63", "e63", "a45", "s63", "g63",
         "sls", "amg gt", "190e", "w201", "w204", "w205", "w206", "w211", "w212", "w213", "w222"});
    addBrand("Audi", "Germany",
        {"audi sport"},
        {"quattro", "rs3", "rs4", "rs5", "rs6", "rs7", "r8", "ttrs", "tt rs", "s3", "s4", "s5", "audi80"});
    addBrand("Volkswagen", "Germany",
        {"vw", "volks", "volkswagon"},
        {"golf", "gti", "golf r", "golf gti", "scirocco", "corrado", "passat", "polo", "beetle"});
    addBrand("Opel", "Germany",
        {"vauxhall"},
        {"corsa", "astra"});
    addBrand("Ruf", "Germany",
        {"ruf automobile"},
        {"ctr", "yellowbird"});
    addBrand("Alpina", "Germany",
        {"bmw alpina"},
        {"b3", "b5"});
    addBrand("Gumpert", "Germany", {}, {"apollo"});
    addBrand("Apollo", "Germany", {"apollo automobil"});
    addBrand("Wiesmann", "Germany");
    addBrand("Borgward", "Germany");
    addBrand("Bitter", "Germany");
    addBrand("Smart", "Germany");
    addBrand("Maybach", "Germany");
    addBrand("Brabus", "Germany");
    addBrand("Auto Union", "Germany");
    addBrand("Artega", "Germany");
    addBrand("Melkus", "Germany");

    // -------------------------------------------------------------
    // Italy
    // -------------------------------------------------------------
    addBrand("Ferrari", "Italy",
        {"scuderia ferrari", "maranello"},
        {"f12", "berlinetta", "488", "458", "f40", "f50",
         "enzo", "laferrari", "sf90", "296", "812", "testarossa", "360 modena", "f430", "portofino", "roma"});
    addBrand("Lamborghini", "Italy",
        {"lambo", "lamborgini"},
        {"aventador", "huracan", "gallardo", "murcielago", "diablo", "countach", "revuelto"});
    addBrand("Alfa Romeo", "Italy",
        {"alfa", "alfa-romeo"},
        {"giulia", "stelvio", "4c", "8c"});
    addBrand("Maserati", "Italy",
        {},
        {"granturismo", "mc12", "mc20", "ghibli"});
    addBrand("Lancia", "Italy",
        {},
        {"delta integrale", "stratos", "fulvia", "037"});
    addBrand("Fiat", "Italy",
        {},
        {"fiat 500", "punto", "multipla"});
    addBrand("Pagani", "Italy",
        {"pagani automobili"},
        {"zonda", "huayra", "utopia"});
    addBrand("Abarth", "Italy",
        {"abarth 500", "abarth 595", "abarth 124", "abarth500", "abarth595"});
    addBrand("De Tomaso", "Italy",
        {"detomaso"},
        {"pantera", "mangusta", "p72"});
    addBrand("Dallara", "Italy", {}, {"stradale"});
    addBrand("Mazzanti", "Italy");
    addBrand("ATS", "Italy", {"ats automobili"});
    addBrand("Bizzarrini", "Italy");
    addBrand("Iso", "Italy", {"iso rivolta"});
    addBrand("Cisitalia", "Italy");
    addBrand("Autobianchi", "Italy");
    addBrand("Bertone", "Italy");
    addBrand("Pininfarina", "Italy", {}, {"battista"});
    addBrand("Italdesign", "Italy");
    addBrand("OSCA", "Italy");
    addBrand("Scuderia Toro Rosso", "Italy", {"toro rosso", "str"});
    addBrand("AlphaTauri", "Italy", {"alpha tauri", "scuderia alphatauri"});
    addBrand("Kimera", "Italy", {"kimera automobili"});
    addBrand("Iveco", "Italy");
    addBrand("Tatuus", "Italy");

    // -------------------------------------------------------------
    // Japan
    // -------------------------------------------------------------
    addBrand("Nissan", "Japan",
        {"datsun", "nismo"},
        {"180sx", "200sx", "240sx", "silvia", "skyline", "gt-r", "gtr",
         "350z", "370z", "fairlady", "stagea", "cefiro", "laurel", "s13", "s14", "s15", "r32", "r33", "r34", "r35"});
    addBrand("Honda", "Japan",
        {"mugen"},
        {"civic", "nsx", "s2000", "integra", "prelude", "crx", "cr-z", "type r", "accord"});
    addBrand("Toyota", "Japan",
        {"gazoo", "gazoo racing"},
        {"supra", "chaser", "cresta", "mark ii", "ae86", "corolla",
         "celica", "mr2", "yaris", "gr86", "gt86", "soarer", "jzx100", "jzx90", "sw20"});
    addBrand("Mazda", "Japan",
        {"mazdaspeed"},
        {"miata", "mx-5", "mx5", "rx-7", "rx7", "rx-8", "rx8", "cosmo", "fd3s", "fc3s", "na6", "na8"});
    addBrand("Subaru", "Japan",
        {"subie"},
        {"impreza", "wrx", "forester", "legacy", "brz"});
    addBrand("Mitsubishi", "Japan",
        {"mitsubishi motors", "ralliart"},
        {"lancer", "eclipse"});
    addBrand("Lexus", "Japan",
        {},
        {"lfa", "is f", "rc f", "lc500"});
    addBrand("Infiniti", "Japan",
        {},
        {"g35", "g37", "q50", "q60"});
    addBrand("Acura", "Japan");
    addBrand("Suzuki", "Japan",
        {},
        {"cappuccino", "swift sport", "jimny"});
    addBrand("Daihatsu", "Japan", {}, {"copen"});
    addBrand("Dome", "Japan");
    addBrand("Tommykaira", "Japan", {"tommy kaira"});
    addBrand("Mitsuoka", "Japan", {}, {"orochi"});
    addBrand("Yamaha", "Japan");
    addBrand("ASL", "Japan", {"autobacs"}, {"garaiya"});
    addBrand("Spoon", "Japan", {"spoon sports"});
    addBrand("Mine's", "Japan", {"mines"});
    addBrand("Top Secret", "Japan");
    addBrand("RE Amemiya", "Japan", {"amemiya"});
    addBrand("VeilSide", "Japan", {"veilside"});
    addBrand("HKS", "Japan");
    addBrand("TOM'S", "Japan", {"toms"});
    addBrand("Isuzu", "Japan");
    addBrand("Scion", "Japan", {}, {"fr-s"});

    // -------------------------------------------------------------
    // United Kingdom
    // -------------------------------------------------------------
    addBrand("Aston Martin", "United Kingdom",
        {"aston", "aston-martin"},
        {"vantage", "db9", "db11", "dbs", "valkyrie", "vulcan"});
    addBrand("McLaren", "United Kingdom",
        {"mclaren automotive"},
        {"f1 gtr", "p1", "senna", "720s", "650s", "570s", "artura"});
    addBrand("Lotus", "United Kingdom",
        {"lotus cars"},
        {"elise", "exige", "evora", "emira", "esprit"});
    addBrand("Jaguar", "United Kingdom",
        {"jag"},
        {"e-type", "f-type", "xj220"});
    addBrand("Bentley", "United Kingdom",
        {},
        {"continental gt"});
    addBrand("Rolls-Royce", "United Kingdom",
        {"rolls royce"},
        {"phantom", "ghost", "wraith"});
    addBrand("Land Rover", "United Kingdom",
        {"landrover"},
        {"range rover", "defender"});
    addBrand("Mini", "United Kingdom",
        {"mini cooper"},
        {"cooper s", "john cooper works", "jcw"});
    addBrand("TVR", "United Kingdom",
        {},
        {"sagaris", "cerbera", "tuscan"});
    addBrand("Caterham", "United Kingdom",
        {},
        {"seven 620r", "superlight"});
    addBrand("Morgan", "United Kingdom",
        {"morgan motor"},
        {"aero 8", "3 wheeler"});
    addBrand("Ariel", "United Kingdom",
        {"ariel motor"},
        {"atom", "nomad"});
    addBrand("Radical", "United Kingdom",
        {"radical motorsport"},
        {"sr3", "sr8", "rxc"});
    addBrand("Ginetta", "United Kingdom",
        {},
        {"g40", "g55", "g60"});
    addBrand("Noble", "United Kingdom",
        {"noble automotive"},
        {"m600"});
    addBrand("Triumph", "United Kingdom",
        {},
        {"spitfire", "tr6"});
    addBrand("MG", "United Kingdom", {"morris garages"});
    addBrand("Austin", "United Kingdom");
    addBrand("Austin-Healey", "United Kingdom", {"austin healey"});
    addBrand("Lister", "United Kingdom");
    addBrand("BAC", "United Kingdom", {"briggs automotive"}, {"mono"});
    addBrand("Ultima", "United Kingdom", {"ultima sports"});
    addBrand("Bowler", "United Kingdom");
    addBrand("Jensen", "United Kingdom", {}, {"interceptor"});
    addBrand("Marcos", "United Kingdom");
    addBrand("Westfield", "United Kingdom");
    addBrand("Prodrive", "United Kingdom");
    addBrand("Gordon Murray", "United Kingdom", {"gordon murray automotive", "gma"});
    addBrand("Vanwall", "United Kingdom");
    addBrand("Force India", "United Kingdom", {"sahara force india", "force india f1"});
    addBrand("Racing Point", "United Kingdom", {"racing point f1"});
    addBrand("Williams", "United Kingdom", {"williams f1", "williams racing"});

    // -------------------------------------------------------------
    // United States
    // -------------------------------------------------------------
    addBrand("Ford", "United States",
        {"ford motor"},
        {"mustang", "gt40", "focus rs", "fiesta st"});
    addBrand("Chevrolet", "United States",
        {"chevy"},
        {"corvette", "camaro", "chevelle", "c6", "c7", "c8", "z06", "zr1", "zl1"});
    addBrand("Dodge", "United States",
        {"mopar"},
        {"viper", "challenger", "charger", "hellcat", "demon"});
    addBrand("Shelby", "United States",
        {"carroll shelby"},
        {"cobra", "daytona coupe"});
    addBrand("Cadillac", "United States",
        {"caddy"},
        {"ct5-v", "ct4-v", "cts-v"});
    addBrand("Pontiac", "United States",
        {},
        {"firebird", "trans am", "solstice"});
    addBrand("Buick", "United States",
        {},
        {"gnx", "grand national"});
    addBrand("Chrysler", "United States",
        {},
        {"300c", "me four-twelve"});
    addBrand("Plymouth", "United States",
        {},
        {"barracuda", "road runner", "superbird"});
    addBrand("Jeep", "United States",
        {},
        {"wrangler", "trackhawk"});
    addBrand("Lincoln", "United States");
    addBrand("Tesla", "United States",
        {},
        {"model s", "model 3", "model x", "model y", "cybertruck", "roadster"});
    addBrand("Saleen", "United States",
        {},
        {"s7", "s281", "s302"});
    addBrand("Hennessey", "United States",
        {"hennessey performance"},
        {"venom f5", "venom gt"});
    addBrand("Panoz", "United States", {}, {"esperante"});
    addBrand("SSC", "United States",
        {"ssc north america", "shelby supercars"},
        {"tuatara", "ultimate aero"});
    addBrand("Vector", "United States",
        {"vector motors"},
        {"w8"});
    addBrand("Rossion", "United States");
    addBrand("Callaway", "United States");
    addBrand("GMC", "United States",
        {},
        {"syclone", "typhoon", "hummer ev"});
    addBrand("AMC", "United States",
        {"american motors"},
        {"javelin", "amx", "gremlin"});
    addBrand("Oldsmobile", "United States", {}, {"442"});
    addBrand("Saturn", "United States");
    addBrand("Mercury", "United States", {}, {"cougar"});
    addBrand("Chaparral", "United States");
    addBrand("Lucid", "United States", {"lucid motors"});
    addBrand("Rivian", "United States", {}, {"r1t", "r1s"});
    addBrand("Fisker", "United States", {}, {"karma"});
    addBrand("Haas", "United States", {"haas f1", "haas f1 team"});
    addBrand("DeLorean Motor Company", "United States", {"delorean", "dmc"});
    addBrand("Scuderia Glickenhaus", "United States", {"scg", "glickenhaus"});
    addBrand("Nilu", "United States", {"nilu27"});
    addBrand("Velocity", "United States");

    // -------------------------------------------------------------
    // France
    // -------------------------------------------------------------
    addBrand("Renault", "France",
        {"renault sport"},
        {"clio rs", "megane rs", "r5 turbo"});
    addBrand("Peugeot", "France",
        {},
        {"205 t16", "208 gti", "908 hdi", "9x8"});
    addBrand("Citroën", "France",
        {"citroen"},
        {"ds3", "xsara", "c4 wrc"});
    addBrand("Bugatti", "France",
        {"bugatti automobiles"},
        {"veyron", "chiron", "eb110", "divo", "bolide", "tourbillon"});
    addBrand("Alpine", "France",
        {"automobiles alpine"},
        {"a110", "a310", "a480", "a521", "a522"});
    addBrand("DS Automobiles", "France");
    addBrand("Venturi", "France", {}, {"atlantique"});
    addBrand("Matra", "France");
    addBrand("Ligier", "France", {}, {"js p3", "js p2"});
    addBrand("Norma", "France", {"norma auto concept"});

    // -------------------------------------------------------------
    // Sweden, Spain, South Korea, Czech Republic, Austria, etc.
    // -------------------------------------------------------------
    addBrand("Koenigsegg", "Sweden",
        {},
        {"ccx", "agera", "one:1", "regera", "jesko", "gemera"});
    addBrand("Volvo", "Sweden",
        {},
        {"850 r", "v70 r"});
    addBrand("Polestar", "Sweden");
    addBrand("Saab", "Sweden");

    addBrand("SEAT", "Spain", {}, {"leon cupra", "ibiza"});
    addBrand("Cupra", "Spain", {}, {"cupra formentor", "cupra leon"});
    addBrand("GTA Spano", "Spain", {"spania gta"});

    addBrand("Hyundai", "South Korea",
        {"hyundai n"},
        {"i30 n", "i20 n", "elantra n", "veloster n", "ioniq 5 n"});
    addBrand("Genesis", "South Korea");
    addBrand("Kia", "South Korea");
    addBrand("Daewoo", "South Korea");

    addBrand("Holden", "Australia", {"hsv", "holden special vehicles"});

    addBrand("Škoda", "Czech Republic", {"skoda"}, {"octavia rs", "fabia rs"});
    addBrand("Praga", "Czech Republic", {}, {"praga r1", "praga bohema"});
    addBrand("Tatra", "Czech Republic");

    addBrand("Red Bull Racing", "Austria", {"red bull"}, {"rb16", "rb18", "rb19", "rb20"});
    addBrand("KTM", "Austria", {}, {"x-bow", "xbow"});

    addBrand("Spyker", "Netherlands", {}, {"c8 aileron", "c8 laviolette"});
    addBrand("Donkervoort", "Netherlands", {}, {"d8 gto"});

    addBrand("Rimac", "Croatia", {"rimac automobili"}, {"concept one", "nevera"});

    addBrand("Sauber", "Switzerland", {"alfa romeo f1", "stake f1"});

    addBrand("Zenvo", "Denmark", {"zenvo automotive"});
    addBrand("Dacia", "Romania");
    addBrand("Lada", "Russia", {"avtovaz", "vaz"});
    addBrand("Tofas", "Turkey");
    addBrand("Devel", "United Arab Emirates", {"devel sixteen"});

    // -------------------------------------------------------------
    // Mod Lore / Fictional names used by RSS, URD, VRC, etc.
    // -------------------------------------------------------------
    m_modLoreAliases["bayro"] = "BMW";             // URD Bayro EGT (BMW M6/M8)
    m_modLoreAliases["ferruccio"] = "Ferrari";     // RSS GT Ferruccio 55 (Ferrari 550)
    m_modLoreAliases["corse"] = "Ferrari";
    m_modLoreAliases["darche"] = "Porsche";        // URD Darche 991/992 (Porsche 911)
    m_modLoreAliases["protech"] = "Porsche";       // RSS Protech
    m_modLoreAliases["pageau"] = "Peugeot";        // VRC Pageau
    m_modLoreAliases["shiro"] = "Nissan";          // URD Shiro (Nissan GT-R)
    m_modLoreAliases["taiko"] = "Toyota";          // URD Taiko
    m_modLoreAliases["amagi"] = "Mazda";           // Modding alias
    m_modLoreAliases["lancaster"] = "Aston Martin";// Mod alias
    m_modLoreAliases["revolution"] = "Chevrolet";  // URD Revolution (Corvette C7.R)
    m_modLoreAliases["vortex"] = "Chevrolet";
    m_modLoreAliases["aura"] = "Audi";             // URD Aura (Audi R8 LMS)
    m_modLoreAliases["lanzo"] = "Lamborghini";     // RSS Lanzo (Huracan)
    m_modLoreAliases["marven"] = "McLaren";        // RSS Marven

    // Sort canonical brands
    std::sort(m_allBrandsSorted.begin(), m_allBrandsSorted.end(), [](const std::string& a, const std::string& b) {
        return a < b;
    });

    // Populate distinct sorted countries
    std::set<std::string> countrySet;
    for (const auto& [key, info] : m_brandLookup) {
        countrySet.insert(info.country);
    }
    for (const auto& [key, info] : m_modelSynonyms) {
        countrySet.insert(info.country);
    }
    m_allCountriesSorted.assign(countrySet.begin(), countrySet.end());

    // Sort rules by pattern length descending so longer/more specific phrases match first
    std::sort(m_sortedBrandRules.begin(), m_sortedBrandRules.end(),
        [](const auto& a, const auto& b) {
            return a.first.size() > b.first.size();
        });
    std::sort(m_sortedModelRules.begin(), m_sortedModelRules.end(),
        [](const auto& a, const auto& b) {
            return a.first.size() > b.first.size();
        });
}

std::string BrandDetector::cleanFolderName(std::string_view folder) {
    std::string s(folder);
    static const std::vector<std::string> prefixes = {
        "ks_", "rss_", "urd_", "vrc_", "ex_", "spt_", "actr_",
        "mnba_", "ddm_", "pe_", "gt_", "bk_", "asrc_", "ac_",
        "cr_", "bm_", "tq_", "sim_", "acfl_", "actk_", "akr_",
        "ag_", "wh_", "tda_", "gta_", "dw_", "prv_", "bt_",
        "dr_", "mkelite_", "exotic_", "street_", "drift_", "race_",
        "01_", "02_", "03_", "04_", "05_", "06_", "07_", "08_", "09_",
        "10_", "11_", "12_", "13_", "14_", "15_", "16_", "17_", "18_", "19_", "20_"};

    std::string lower = toLower(s);
    for (const auto& prefix : prefixes) {
        if (lower.starts_with(prefix)) {
            s = s.substr(prefix.size());
            break;
        }
    }

    return normalizeText(s);
}

std::optional<std::string> BrandDetector::getCountryForBrand(std::string_view brand) const {
    const std::string lower = toLower(brand);
    auto it = m_brandLookup.find(lower);
    if (it != m_brandLookup.end()) {
        return it->second.country;
    }
    auto it2 = m_modelSynonyms.find(lower);
    if (it2 != m_modelSynonyms.end()) {
        return it2->second.country;
    }
    return std::nullopt;
}

const std::vector<std::string>& BrandDetector::getKnownBrands() const {
    return m_allBrandsSorted;
}

const std::vector<std::string>& BrandDetector::getKnownCountries() const {
    return m_allCountriesSorted;
}

int BrandDetector::levenshteinDistance(std::string_view s1, std::string_view s2) {
    const size_t m = s1.size();
    const size_t n = s2.size();
    if (m == 0) return static_cast<int>(n);
    if (n == 0) return static_cast<int>(m);

    std::vector<int> prev(n + 1);
    std::vector<int> curr(n + 1);

    for (size_t j = 0; j <= n; ++j) prev[j] = static_cast<int>(j);

    for (size_t i = 1; i <= m; ++i) {
        curr[0] = static_cast<int>(i);
        for (size_t j = 1; j <= n; ++j) {
            const int cost = (std::tolower(static_cast<unsigned char>(s1[i - 1])) ==
                              std::tolower(static_cast<unsigned char>(s2[j - 1]))) ? 0 : 1;
            curr[j] = std::min({curr[j - 1] + 1, prev[j] + 1, prev[j - 1] + cost});
        }
        prev = curr;
    }
    return curr[n];
}

// Helper: perform greedy substring match on a set of sorted rules
static std::optional<BrandDetector::BrandInfo> matchRules(
    std::string_view lower,
    const std::vector<std::pair<std::string, BrandDetector::BrandInfo>>& sortedRules) {
    
    for (const auto& [key, info] : sortedRules) {
        if (key.size() < 3) continue;
        size_t pos = lower.find(key);
        while (pos != std::string::npos) {
            bool leftOk = (pos == 0 || !std::isalnum(static_cast<unsigned char>(lower[pos - 1])));
            bool rightOk = (pos + key.size() == lower.size() || !std::isalnum(static_cast<unsigned char>(lower[pos + key.size()])));
            if (leftOk && rightOk) {
                return info;
            }
            pos = lower.find(key, pos + 1);
        }
    }
    return std::nullopt;
}

std::optional<BrandDetector::BrandInfo> BrandDetector::matchBrandName(std::string_view text) const {
    if (text.empty()) return std::nullopt;

    const std::string lower = normalizeText(text);

    // 1. Direct whole-string match against brand names
    auto it = m_brandLookup.find(lower);
    if (it != m_brandLookup.end()) {
        return it->second;
    }

    // 2. Mod lore aliases (e.g. ferruccio -> Ferrari, bayro -> BMW)
    for (const auto& [alias, targetBrand] : m_modLoreAliases) {
        size_t pos = lower.find(alias);
        while (pos != std::string::npos) {
            bool leftOk = (pos == 0 || !std::isalnum(static_cast<unsigned char>(lower[pos - 1])));
            bool rightOk = (pos + alias.size() == lower.size() || !std::isalnum(static_cast<unsigned char>(lower[pos + alias.size()])));
            if (leftOk && rightOk) {
                auto targetIt = m_brandLookup.find(toLower(targetBrand));
                if (targetIt != m_brandLookup.end()) {
                    return targetIt->second;
                }
            }
            pos = lower.find(alias, pos + 1);
        }
    }

    // 3. Greedy sorted brand rule match (longer names like "Alfa Romeo" before "Alfa")
    return matchRules(lower, m_sortedBrandRules);
}

std::optional<BrandDetector::BrandInfo> BrandDetector::matchModelName(std::string_view text) const {
    if (text.empty()) return std::nullopt;

    const std::string lower = normalizeText(text);

    // 1. Direct whole-string match against model synonyms
    auto it = m_modelSynonyms.find(lower);
    if (it != m_modelSynonyms.end()) {
        return it->second;
    }

    // 2. Greedy sorted model rule match
    return matchRules(lower, m_sortedModelRules);
}

std::optional<BrandDetector::BrandInfo> BrandDetector::matchText(std::string_view text) const {
    // Try brand names first (primary), then model synonyms (fallback)
    auto brandMatch = matchBrandName(text);
    if (brandMatch.has_value()) return brandMatch;
    return matchModelName(text);
}

std::optional<BrandDetector::BrandInfo> BrandDetector::matchFuzzy(std::string_view token) const {
    // Only allow fuzzy matching on words that are at least 6 characters long
    if (token.size() < 6) return std::nullopt;

    const std::string lower = toLower(token);

    // Skip common car words, mod descriptors, and countries
    if (s_ignoredFuzzyWords.contains(lower)) {
        return std::nullopt;
    }

    int bestDist = 2; // Maximum edit distance allowed: 1
    const BrandInfo* bestInfo = nullptr;

    for (const auto& [key, info] : m_brandLookup) {
        if (key.size() < 6) continue;
        int lenDiff = std::abs(static_cast<int>(key.size()) - static_cast<int>(lower.size()));
        if (lenDiff > 1) continue;

        int dist = levenshteinDistance(lower, key);
        if (dist <= 1 && dist < bestDist) {
            bestDist = dist;
            bestInfo = &info;
        }
    }

    if (bestInfo) {
        return *bestInfo;
    }
    return std::nullopt;
}

bool BrandDetector::isBrandInText(std::string_view brand, std::string_view text) const {
    if (brand.empty() || text.empty()) return false;
    const std::string bNorm = normalizeText(brand);
    const std::string tNorm = normalizeText(text);
    if (bNorm.empty() || tNorm.empty()) return false;

    // Direct substring match with word boundaries
    size_t pos = tNorm.find(bNorm);
    while (pos != std::string::npos) {
        bool leftOk = (pos == 0 || !std::isalnum(static_cast<unsigned char>(tNorm[pos - 1])));
        bool rightOk = (pos + bNorm.size() == tNorm.size() || !std::isalnum(static_cast<unsigned char>(tNorm[pos + bNorm.size()])));
        if (leftOk && rightOk) return true;
        pos = tNorm.find(bNorm, pos + 1);
    }

    // Also check if canonical name of brand is in text (e.g. Chevy in text when brand is Chevrolet)
    auto it = m_brandLookup.find(toLower(brand));
    if (it != m_brandLookup.end()) {
        const std::string canonNorm = normalizeText(it->second.canonicalName);
        if (!canonNorm.empty() && canonNorm != bNorm) {
            size_t cPos = tNorm.find(canonNorm);
            while (cPos != std::string::npos) {
                bool leftOk = (cPos == 0 || !std::isalnum(static_cast<unsigned char>(tNorm[cPos - 1])));
                bool rightOk = (cPos + canonNorm.size() == tNorm.size() || !std::isalnum(static_cast<unsigned char>(tNorm[cPos + canonNorm.size()])));
                if (leftOk && rightOk) return true;
                cPos = tNorm.find(canonNorm, cPos + 1);
            }
        }
    }

    return false;
}

DetectionResult BrandDetector::detect(const CarItem& car) const {
    DetectionResult result;

    const std::string currentBrandLower = toLower(car.brand);
    const std::string currentCountryLower = toLower(car.country);

    // -------------------------------------------------------------
    // RULE 1: Compare car name & folder with brand!
    // Use BRAND NAME matching only (not model synonyms) to avoid
    // false positives like "stinger" matching Kia for Chevy Yenko Stinger
    // -------------------------------------------------------------
    if (!car.isBrandMissing()) {
        // 1a. If current brand is already confirmed by car name or folder:
        if (isBrandInText(car.brand, car.name) || isBrandInText(car.brand, car.folderName)) {
            return result; // matched = false (100% Verified!)
        }

        // 1b. Current brand is not in name or folder.
        // Check if car name (or clean folder) explicitly has a DIFFERENT known brand NAME
        auto nameMatch = matchBrandName(car.name);
        if (!nameMatch.has_value()) {
            std::string cleanedFolder = cleanFolderName(car.folderName);
            nameMatch = matchBrandName(cleanedFolder);
        }

        if (nameMatch.has_value()) {
            const auto& detected = *nameMatch;
            const bool brandMatches = (toLower(detected.canonicalName) == currentBrandLower);

            if (!brandMatches) {
                // BRAND MISMATCH!
                result.brand = detected.canonicalName;
                result.country = detected.country;
                result.confidence = "High";
                result.reason = "Brand mismatch: car name indicates '" + detected.canonicalName + 
                                "', but brand in file is '" + car.brand + "'";
                result.matched = true;
                return result;
            }
        }

        // 1c. Neither car name nor folder indicated any conflicting known brand name.
        // Custom mod brand or model-only match. Leave Verified!
        return result; // matched = false (Verified!)
    }

    // -------------------------------------------------------------
    // RULE 2: Inspect car 'name' property (Priority match)
    // Example: "Twin Turbo Ferrari F12 Berlinetta" -> Ferrari (Italy)
    // -------------------------------------------------------------
    if (!car.name.empty()) {
        auto nameMatch = matchText(car.name);
        if (nameMatch.has_value()) {
            const auto& info = *nameMatch;
            const bool brandMatches = (currentBrandLower == toLower(info.canonicalName));

            if (brandMatches) {
                // Already matches! Not a problem!
                return result;
            }

            result.brand = info.canonicalName;
            result.country = info.country;
            result.confidence = "High";
            result.reason = "Detected '" + info.canonicalName + "' from car name: '" + car.name + "'";
            result.matched = true;
            return result;
        }
    }

    // -------------------------------------------------------------
    // RULE 3: Inspect cleaned mod folder name
    // Example: "01_ferrari_f12_berlinetta_mkelite" -> Ferrari (Italy)
    // -------------------------------------------------------------
    std::string cleanedFolder = cleanFolderName(car.folderName);
    auto folderMatch = matchText(cleanedFolder);
    if (folderMatch.has_value()) {
        const auto& info = *folderMatch;
        const bool brandMatches = (currentBrandLower == toLower(info.canonicalName));

        if (brandMatches) {
            return result;
        }

        result.brand = info.canonicalName;
        result.country = info.country;
        result.confidence = "High";
        result.reason = "Detected '" + info.canonicalName + "' from folder name: '" + car.folderName + "'";
        result.matched = true;
        return result;
    }

    // -------------------------------------------------------------
    // RULE 3b: Position-weighted folder token inspection
    // If the full folder name didn't trigger, inspect individual tokens and pairs
    // (e.g. "akr_ford_raptor" -> first token "ford" or pair "ford raptor")
    // -------------------------------------------------------------
    auto folderTokens = splitWords(cleanedFolder);
    for (size_t i = 0; i < folderTokens.size(); ++i) {
        const auto& token = folderTokens[i];
        if (token.size() < 2) continue;

        // Check single token (e.g. "ford", "toyota", "mb", "s13", "e36", "raptor")
        auto tokenMatch = matchText(token);
        if (tokenMatch.has_value()) {
            const auto& info = *tokenMatch;
            const bool brandMatches = (currentBrandLower == toLower(info.canonicalName));

            if (brandMatches) {
                return result;
            }

            result.brand = info.canonicalName;
            result.country = info.country;
            result.confidence = (i == 0) ? "High" : "Medium";
            result.reason = "Detected '" + info.canonicalName + "' from folder segment: '" + token + "'";
            result.matched = true;
            return result;
        }

        // Check 2-token window (e.g. "alfa romeo", "aston martin", "ford raptor")
        if (i + 1 < folderTokens.size()) {
            std::string pair = token + " " + folderTokens[i + 1];
            auto pairMatch = matchText(pair);
            if (pairMatch.has_value()) {
                const auto& info = *pairMatch;
                const bool brandMatches = (currentBrandLower == toLower(info.canonicalName));

                if (brandMatches) {
                    return result;
                }

                result.brand = info.canonicalName;
                result.country = info.country;
                result.confidence = "High";
                result.reason = "Detected '" + info.canonicalName + "' from folder phrase: '" + pair + "'";
                result.matched = true;
                return result;
            }
        }
    }

    // -------------------------------------------------------------
    // RULE 3c: Inspect car 'tags'
    // -------------------------------------------------------------
    for (const auto& tag : car.tags) {
        if (tag.empty()) continue;
        auto tagMatch = matchText(tag);
        if (tagMatch.has_value()) {
            const auto& info = *tagMatch;
            const bool brandMatches = (currentBrandLower == toLower(info.canonicalName));

            if (brandMatches) {
                return result;
            }

            result.brand = info.canonicalName;
            result.country = info.country;
            result.confidence = "High";
            result.reason = "Detected '" + info.canonicalName + "' from car tag: '" + tag + "'";
            result.matched = true;
            return result;
        }
    }

    // -------------------------------------------------------------
    // RULE 4: Strict Typo matching (ONLY if brand is still missing)
    // -------------------------------------------------------------
    if (car.isBrandMissing()) {
        auto nameWords = splitWords(car.name);
        for (const auto& word : nameWords) {
            auto fuzzy = matchFuzzy(word);
            if (fuzzy.has_value()) {
                const auto& info = *fuzzy;
                result.brand = info.canonicalName;
                result.country = info.country;
                result.confidence = "Medium";
                result.reason = "Typo matched '" + word + "' to '" + info.canonicalName + "'";
                result.matched = true;
                return result;
            }
        }
    }

    return result;
}

void BrandDetector::registerDiscoveredBrand(const std::string& brand, const std::string& country, const std::string& badgePath) {
    if (brand.empty() || brand == "Brand Not Found" || brand == "Unknown") return;

    std::string lowerBrand = toLower(brand);

    // If we have a badge path, save it in the dynamic badge map
    if (!badgePath.empty()) {
        m_brandBadges[lowerBrand] = badgePath;
    }

    // Check if brand already exists in database
    auto it = m_brandLookup.find(lowerBrand);
    if (it != m_brandLookup.end()) {
        if (it->second.country.empty() && !country.empty() && country != "Unknown") {
            it->second.country = country;
        }
        return;
    }

    // Brand is new from user's library: register it dynamically!
    BrandInfo info{brand, country};
    m_brandLookup[lowerBrand] = info;
    m_sortedBrandRules.push_back({lowerBrand, info});

    // Keep sorted brand rules sorted by pattern length descending
    std::stable_sort(m_sortedBrandRules.begin(), m_sortedBrandRules.end(),
        [](const auto& a, const auto& b) {
            return a.first.size() > b.first.size();
        });

    // Add to allBrandsSorted if not present
    if (std::find(m_allBrandsSorted.begin(), m_allBrandsSorted.end(), brand) == m_allBrandsSorted.end()) {
        m_allBrandsSorted.push_back(brand);
        std::sort(m_allBrandsSorted.begin(), m_allBrandsSorted.end(), [](const std::string& a, const std::string& b) {
            return std::lexicographical_compare(
                a.begin(), a.end(), b.begin(), b.end(),
                [](char c1, char c2) { return std::tolower(static_cast<unsigned char>(c1)) < std::tolower(static_cast<unsigned char>(c2)); }
            );
        });
    }

    // Add country if valid
    if (!country.empty() && country != "Unknown") {
        if (std::find(m_allCountriesSorted.begin(), m_allCountriesSorted.end(), country) == m_allCountriesSorted.end()) {
            m_allCountriesSorted.push_back(country);
            std::sort(m_allCountriesSorted.begin(), m_allCountriesSorted.end());
        }
    }
}

void BrandDetector::loadBrandBadgesFromSystem() {
    std::vector<std::filesystem::path> badgeDirs;

    // 1. Content Manager AppData directories
    char* localAppBuf = nullptr;
    size_t localAppLen = 0;
    if (_dupenv_s(&localAppBuf, &localAppLen, "LOCALAPPDATA") == 0 && localAppBuf) {
        std::filesystem::path base(localAppBuf);
        badgeDirs.push_back(base / "AcTools Content Manager" / "Data" / "Brand Badges");
        badgeDirs.push_back(base / "AcTools Content Manager" / "Data (User)" / "Brand Badges");
        free(localAppBuf);
    }

    char* userProfBuf = nullptr;
    size_t userProfLen = 0;
    if (_dupenv_s(&userProfBuf, &userProfLen, "USERPROFILE") == 0 && userProfBuf) {
        std::filesystem::path base(userProfBuf);
        badgeDirs.push_back(base / "AppData" / "Local" / "AcTools Content Manager" / "Data" / "Brand Badges");
        badgeDirs.push_back(base / "AppData" / "Local" / "AcTools Content Manager" / "Data (User)" / "Brand Badges");
        free(userProfBuf);
    }

    // 2. Scan all discovered badge directories
    std::error_code ec;
    for (const auto& dir : badgeDirs) {
        if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec)) {
            continue;
        }

        try {
            for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
                if (ec || !entry.is_regular_file(ec)) continue;

                std::string ext = entry.path().extension().string();
                std::string lowerExt = toLower(ext);
                if (lowerExt != ".png" && lowerExt != ".jpg" && lowerExt != ".jpeg" && lowerExt != ".svg") {
                    continue;
                }

                std::string stem = entry.path().stem().string();
                std::string fullPath = entry.path().string();
                std::string lowerStem = toLower(stem);
                std::string normStem = normalizeText(stem);

                // Register lower and normalized stem
                m_brandBadges[lowerStem] = fullPath;
                m_brandBadges[normStem] = fullPath;

                // Also if this matches a canonical brand in our database, register canonical name
                auto it = m_brandLookup.find(normStem);
                if (it != m_brandLookup.end()) {
                    m_brandBadges[toLower(it->second.canonicalName)] = fullPath;
                }
                auto it2 = m_brandLookup.find(lowerStem);
                if (it2 != m_brandLookup.end()) {
                    m_brandBadges[toLower(it2->second.canonicalName)] = fullPath;
                }
            }
        } catch (...) {
            // Ignore directory read exceptions
        }
    }
}

std::string BrandDetector::getBadgeForBrand(std::string_view brand) const {
    if (brand.empty()) return "";

    // 1. Direct lowercase lookup
    std::string lower = toLower(brand);
    auto it = m_brandBadges.find(lower);
    if (it != m_brandBadges.end()) {
        return it->second;
    }

    // 2. Normalized text lookup
    std::string norm = normalizeText(brand);
    auto itNorm = m_brandBadges.find(norm);
    if (itNorm != m_brandBadges.end()) {
        return itNorm->second;
    }

    // 3. Match against canonical brand
    auto brandMatch = matchBrandName(brand);
    if (brandMatch.has_value()) {
        std::string canonLower = toLower(brandMatch->canonicalName);
        auto itCanon = m_brandBadges.find(canonLower);
        if (itCanon != m_brandBadges.end()) {
            return itCanon->second;
        }
        std::string canonNorm = normalizeText(brandMatch->canonicalName);
        auto itCanonNorm = m_brandBadges.find(canonNorm);
        if (itCanonNorm != m_brandBadges.end()) {
            return itCanonNorm->second;
        }
    }

    return "";
}

} // namespace acbo
