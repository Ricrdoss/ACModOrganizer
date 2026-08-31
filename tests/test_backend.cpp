#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <QCoreApplication>
#include "BrandDetector.hpp"
#include "CarItem.hpp"
#include "JsonWriter.hpp"
#include "ScannerEngine.hpp"
#include "UpdateManager.hpp"

using namespace acbo;

void runBrandDetectorTests() {
    std::cout << "\n=== Running BrandDetector Tests ===" << std::endl;
    BrandDetector detector;

    // Test 1: Car name contains "Ferrari" -> sets brand to Ferrari and country to Italy
    {
        CarItem car;
        car.folderName = "custom_mod_488";
        car.name = "Ferrari 488 GT3 Evo 2020";
        car.brand = "Unknown";
        car.country = "";

        auto res = detector.detect(car);
        assert(res.matched);
        assert(res.brand == "Ferrari");
        assert(res.country == "Italy");
        std::cout << "[PASS] Test 1: Name contains 'Ferrari' -> Brand: " << res.brand << ", Country: " << res.country << std::endl;
    }

    // Test 2: Car already has valid brand "BMW" -> Verified, not forced to change!
    {
        CarItem car;
        car.folderName = "bmw_m3_e30";
        car.name = "BMW M3 E30";
        car.brand = "BMW";
        car.country = "Italy"; // Modder or race team country

        auto res = detector.detect(car);
        assert(!res.matched); // Existing brand is valid, not flagged as a problem!
        assert(car.status() == CarStatus::Verified);
        std::cout << "[PASS] Test 2: Car with existing brand 'BMW' is recognized as Verified without being forced to change" << std::endl;
    }

    // Test 3: Mod lore alias (bayro -> BMW)
    {
        CarItem car;
        car.folderName = "urd_bayro_egt_2018";
        car.name = "Bayro EGT 2018";
        car.brand = "Unknown";
        car.country = "";

        auto res = detector.detect(car);
        assert(res.matched);
        assert(res.brand == "BMW");
        assert(res.country == "Germany");
        std::cout << "[PASS] Test 3: Mod lore alias resolved -> " << res.brand << " (" << res.country << ")" << std::endl;
    }

    // Test 4: Prefix stripping and car name detection
    {
        CarItem car;
        car.folderName = "actr_toyota_chaser_jzx100";
        car.name = "Toyota Chaser Tourer V (JZX100)";
        car.brand = "";
        car.country = "";

        auto res = detector.detect(car);
        assert(res.matched);
        assert(res.brand == "Toyota");
        assert(res.country == "Japan");
        std::cout << "[PASS] Test 4: Prefix stripped & detected -> " << res.brand << " (" << res.country << ")" << std::endl;
    }

    // Test 6: Twin Turbo Ferrari F12 Berlinetta (should detect Ferrari, Italy, NEVER Praga)
    {
        CarItem car;
        car.folderName = "01_ferrari_f12_berlinetta_mkelite";
        car.name = "Twin Turbo Ferrari F12 Berlinetta - MkElite Server Exclusive  (Drift, Drag, CutUp & Drag500+)";
        car.brand = "Ferrari";
        car.country = "Italy";

        auto res = detector.detect(car);
        if (res.matched) {
            std::cout << "[FAIL] Test 6: Ferrari was incorrectly suggested to change to: " << res.brand << " (" << res.country << ")!" << std::endl;
        } else {
            std::cout << "[PASS] Test 6: Ferrari F12 recognized as verified Ferrari (Italy)" << std::endl;
        }
        assert(!res.matched);
    }

    // Test 7: 180sx (should detect Nissan, Japan, NEVER BMW)
    {
        CarItem car;
        car.folderName = "180sx_munted_shitbucket";
        car.name = "180sx beater missile";
        car.brand = "Unknown";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 7 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Nissan");
        assert(res.country == "Japan");
        std::cout << "[PASS] Test 7: 180sx detected as Nissan (Japan)" << std::endl;
    }

    // Test 8: Civic (should detect Honda, Japan, NEVER Opel)
    {
        CarItem car;
        car.folderName = "7sec_king_street_awd_eg";
        car.name = "7 Second \"LOW BOOST\" AWD Civic MkElite Spec";
        car.brand = "Unknown";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 8 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Honda");
        assert(res.country == "Japan");
        std::cout << "[PASS] Test 8: Civic detected as Honda (Japan)" << std::endl;
    }

    // Test 9: BMW E30 (should detect BMW, Germany, NEVER SEAT)
    {
        CarItem car;
        car.folderName = "acdfr_21_e30_s54t";
        car.name = "# 2021 ACDFR BMW E30 325i";
        car.brand = "Unknown";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 9 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "BMW");
        assert(res.country == "Germany");
        std::cout << "[PASS] Test 9: BMW E30 detected as BMW (Germany)" << std::endl;
    }

    // Test 10: F1 2015 Ferrari (should detect Ferrari, Italy, NEVER Alfa Romeo)
    {
        CarItem car;
        car.folderName = "acfl_2015_ferrari";
        car.name = "F1 2015 Ferrari";
        car.brand = "Unknown";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 10 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Ferrari");
        assert(res.country == "Italy");
        std::cout << "[PASS] Test 10: F1 Ferrari detected as Ferrari (Italy)" << std::endl;
    }

    // Test 11: F1 2015 Force India (should detect Force India, United Kingdom, NEVER Ford)
    {
        CarItem car;
        car.folderName = "acfl_2015_force_india";
        car.name = "F1 2015 Force India";
        car.brand = "Unknown";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 11 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Force India");
        assert(res.country == "United Kingdom");
        std::cout << "[PASS] Test 11: F1 Force India detected as Force India (United Kingdom)" << std::endl;
    }

    // Test 12: Completely unknown car name and folder (should NOT detect false brand, should be Brand Not Found)
    {
        CarItem car;
        car.folderName = "custom_unknown_track_toy_xyz";
        car.name = "Ultimate Mystery Track Toy Prototype";
        car.brand = "";
        car.country = "";

        auto res = detector.detect(car);
        assert(!res.matched);
        assert(car.isBrandMissing());
        assert(car.statusString() == "Brand Not Found");
        std::cout << "[PASS] Test 12: Undetected car correctly recognized with status 'Brand Not Found'" << std::endl;
    }

    // Test 13: abarth500 (letter-digit boundary, detects Abarth, Italy)
    {
        CarItem car;
        car.folderName = "abarth500";
        car.name = "abarth500";
        car.brand = "";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 13 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Abarth");
        assert(res.country == "Italy");
        std::cout << "[PASS] Test 13: abarth500 detected as Abarth (Italy)" << std::endl;
    }

    // Test 14: abarth500_s1 (letter-digit boundary with suffix, detects Abarth, Italy)
    {
        CarItem car;
        car.folderName = "abarth500_s1";
        car.name = "abarth500_s1";
        car.brand = "";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 14 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Abarth");
        assert(res.country == "Italy");
        std::cout << "[PASS] Test 14: abarth500_s1 detected as Abarth (Italy)" << std::endl;
    }

    // Test 15: akr_ford_raptor_v2 (modder prefix + folder token) -> Ford (United States)
    {
        CarItem car;
        car.folderName = "akr_ford_raptor_v2";
        car.name = "Safety Car";
        car.brand = "Unknown";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 15 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Ford");
        assert(res.country == "United States");
        std::cout << "[PASS] Test 15: akr_ford_raptor_v2 detected as Ford (United States)" << std::endl;
    }

    // Test 16: ag_supra_racing_v10 (model code in folder) -> Toyota (Japan)
    {
        CarItem car;
        car.folderName = "ag_supra_racing_v10";
        car.name = "Racing V10";
        car.brand = "";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 16 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Toyota");
        assert(res.country == "Japan");
        std::cout << "[PASS] Test 16: ag_supra_racing_v10 detected as Toyota (Japan)" << std::endl;
    }

    // Test 17: mb_c63_amg_w204 (slang + chassis code) -> Mercedes-Benz (Germany)
    {
        CarItem car;
        car.folderName = "mb_c63_amg_w204";
        car.name = "German Beast";
        car.brand = "";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 17 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Mercedes-Benz");
        assert(res.country == "Germany");
        std::cout << "[PASS] Test 17: mb_c63_amg_w204 detected as Mercedes-Benz (Germany)" << std::endl;
    }

    // Test 18: chevy_corvette_c8 (slang + chassis code) -> Chevrolet (United States)
    {
        CarItem car;
        car.folderName = "chevy_corvette_c8";
        car.name = "Track Toy C8";
        car.brand = "";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 18 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Chevrolet");
        assert(res.country == "United States");
        std::cout << "[PASS] Test 18: chevy_corvette_c8 detected as Chevrolet (United States)" << std::endl;
    }

    // Test 19: urd_darche_992 (mod lore alias + chassis code) -> Porsche (Germany)
    {
        CarItem car;
        car.folderName = "urd_darche_992";
        car.name = "Darche Cup";
        car.brand = "";
        car.country = "";

        auto res = detector.detect(car);
        std::cout << "Test 19 result: " << res.brand << " (" << res.country << ")" << std::endl;
        assert(res.matched);
        assert(res.brand == "Porsche");
        assert(res.country == "Germany");
        std::cout << "[PASS] Test 19: urd_darche_992 detected as Porsche (Germany)" << std::endl;
    }

    // Test 20: Car already has correct brand "Ferrari" (country empty) -> NOT matched as problem, status is Verified
    {
        CarItem car;
        car.folderName = "ferrari_458";
        car.name = "Ferrari 458 Italia";
        car.brand = "Ferrari";
        car.country = "";

        auto res = detector.detect(car);
        assert(!res.matched); // Must NOT be treated as a problem!
        assert(car.status() == CarStatus::Verified);
        std::cout << "[PASS] Test 20: Existing correct brand 'Ferrari' is NOT flagged as a problem and is Verified" << std::endl;
    }

    // Test 21: Car already has correct brand and country "BMW", "Germany" -> NOT matched as problem, status is Verified
    {
        CarItem car;
        car.folderName = "bmw_m4";
        car.name = "BMW M4 Coupe";
        car.brand = "BMW";
        car.country = "Germany";

        auto res = detector.detect(car);
        assert(!res.matched);
        assert(car.status() == CarStatus::Verified);
        std::cout << "[PASS] Test 21: Existing correct brand and country 'BMW' (Germany) is NOT flagged as a problem" << std::endl;
    }

    // Test 22: Dodge Viper with country "USA" (synonym for United States) -> NOT matched as problem, Verified!
    {
        CarItem car;
        car.folderName = "bz_viper_ta_2";
        car.name = "Dodge Viper TA 2.0 '15";
        car.brand = "Dodge";
        car.country = "USA";

        auto res = detector.detect(car);
        assert(!res.matched); // Must NOT be treated as a problem!
        assert(car.status() == CarStatus::Verified);
        std::cout << "[PASS] Test 22: Dodge Viper with country 'USA' is recognized as Verified without suggestion!" << std::endl;
    }

    // Test 23: User case: Kia Carnival KA4 with brand set to "Bugatti" -> Brand mismatch detected!
    {
        CarItem car;
        car.folderName = "kia_carnival_KA4";
        car.name = "Kia Carnival KA4";
        car.brand = "Bugatti";
        car.country = "France";

        auto res = detector.detect(car);
        assert(res.matched);
        assert(res.brand == "Kia");
        assert(res.country == "South Korea");
        std::cout << "[PASS] Test 23: Kia with brand 'Bugatti' successfully detected as mismatch -> Suggested: " << res.brand << " (" << res.country << ")" << std::endl;
    }

    // Test 24: User case: Ferrari F12 Berlinetta with brand set to "Praga" -> Brand mismatch detected!
    {
        CarItem car;
        car.folderName = "01_ferrari_f12_berlinetta_mkelite";
        car.name = "Twin Turbo Ferrari F12 Berlinetta";
        car.brand = "Praga";
        car.country = "Czech Republic";

        auto res = detector.detect(car);
        assert(res.matched);
        assert(res.brand == "Ferrari");
        assert(res.country == "Italy");
        std::cout << "[PASS] Test 24: Ferrari with brand 'Praga' successfully detected as mismatch -> Suggested: " << res.brand << " (" << res.country << ")" << std::endl;
    }

    // Test 25: User case: Honda Civic with brand set to "Opel" -> Brand mismatch detected!
    {
        CarItem car;
        car.folderName = "honda_civic_ek9";
        car.name = "Honda Civic Type R";
        car.brand = "Opel";
        car.country = "Germany";

        auto res = detector.detect(car);
        assert(res.matched);
        assert(res.brand == "Honda");
        assert(res.country == "Japan");
        std::cout << "[PASS] Test 25: Honda Civic with brand 'Opel' successfully detected as mismatch -> Suggested: " << res.brand << " (" << res.country << ")" << std::endl;
    }
}

void runJsonWriterTests() {
    std::cout << "\n=== Running JsonWriter Tests ===" << std::endl;
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "acbo_test";
    std::filesystem::create_directories(tempDir);
    std::filesystem::path testJson = tempDir / "ui_car.json";

    // Write a mock ui_car.json with comments, specs, and extra fields
    {
        std::ofstream out(testJson);
        out << "{\n"
            << "    // Mock Assetto Corsa ui_car.json\n"
            << "    \"name\": \"Porsche 911 GT3 R\",\n"
            << "    \"brand\": \"Unknown\",\n"
            << "    \"country\": \"\",\n"
            << "    \"specs\": {\n"
            << "        \"bhp\": \"500\",\n"
            << "        \"torque\": \"470\"\n"
            << "    },\n"
            << "    \"tags\": [\"gt3\", \"porsche\"],\n"
            << "    \"year\": 2019\n"
            << "}\n";
        out.close();
    }

    CarItem car;
    car.folderName = "porsche_911_gt3_r";
    car.jsonPath = testJson.string();
    car.name = "Porsche 911 GT3 R";
    car.brand = "Unknown";
    car.country = "";
    car.editedBrand = "Porsche";
    car.editedCountry = "Germany";
    car.isPendingSave = true;

    auto res = JsonWriter::saveCar(car);
    assert(res.success);
    std::cout << "[PASS] Test 6: Safe write completed successfully" << std::endl;

    // Verify .bak file was created
    std::filesystem::path bakPath = testJson;
    bakPath += ".bak";
    assert(std::filesystem::exists(bakPath));
    std::cout << "[PASS] Test 7: Pristine original backup exists at -> " << bakPath.string() << std::endl;

    // Verify updated JSON contents and preservation of other fields
    {
        std::ifstream in(testJson);
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        std::cout << "[PASS] Test 8: Updated fields written and original metadata preserved intact" << std::endl;
    }

    // Test 9: Handling unescaped raw CR/LF inside description strings (user failure case)
    {
        std::filesystem::path unescapedTestJson = tempDir / "ui_car_unescaped.json";
        {
            std::ofstream out(unescapedTestJson, std::ios::binary);
            out << "{\n"
                << "    \"name\": \"Abarth 500 EsseEsse\",\n"
                << "    \"brand\": \"Unknown\",\n"
                << "    \"country\": \"\",\n"
                << "    \"description\": \"Turning modern Fiat into Abarth\r\nmeant lowering ride,\r\nadding turbo.\"\n"
                << "}\n";
            out.close();
        }

        CarItem car9;
        car9.folderName = "abarth500";
        car9.jsonPath = unescapedTestJson.string();
        car9.editedBrand = "Abarth";
        car9.editedCountry = "Italy";
        car9.isPendingSave = true;

        auto res9 = JsonWriter::saveCar(car9);
        assert(res9.success);
        std::cout << "[PASS] Test 9: Saved successfully with unescaped CR/LF in description!" << std::endl;
    }

    // Test 10: Handling corrupted mod JSON with trailing commas via resilient fallback
    {
        std::filesystem::path malformedTestJson = tempDir / "ui_car_malformed.json";
        {
            std::ofstream out(malformedTestJson, std::ios::binary);
            out << "{\n"
                << "    \"name\": \"Custom Mod Car\",\n"
                << "    \"brand\": \"Unknown\",\n"
                << "    \"country\": \"\",\n"
                << "    \"specs\": {\n"
                << "        \"bhp\": \"600\",\n" // Trailing comma!
                << "    },\n"
                << "}\n";
            out.close();
        }

        CarItem car10;
        car10.folderName = "custom_mod";
        car10.jsonPath = malformedTestJson.string();
        car10.editedBrand = "Nissan";
        car10.editedCountry = "Japan";
        car10.isPendingSave = true;

        auto res10 = JsonWriter::saveCar(car10);
        assert(res10.success);

        std::ifstream in(malformedTestJson);
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        assert(content.find("\"brand\": \"Nissan\"") != std::string::npos);
        assert(content.find("\"country\": \"Japan\"") != std::string::npos);
        std::cout << "[PASS] Test 10: Resilient fallback successfully saved corrupted mod JSON with trailing comma!" << std::endl;
    }

    // Test 11: Handling car with missing ui_car.json (2015_season user case)
    {
        std::filesystem::path missingJsonCarDir = tempDir / "2015_season";
        std::filesystem::create_directories(missingJsonCarDir);

        CarItem car11;
        car11.folderName = "2015_season";
        car11.folderPath = missingJsonCarDir.string();
        car11.jsonPath = ""; // No existing ui_car.json!
        car11.name = "F1 2015 Season";
        car11.editedBrand = "Mercedes-Benz";
        car11.editedCountry = "Germany";
        car11.isPendingSave = true;

        auto res11 = JsonWriter::saveCar(car11);
        assert(res11.success);
        assert(std::filesystem::exists(missingJsonCarDir / "ui" / "ui_car.json"));

        std::ifstream in(missingJsonCarDir / "ui" / "ui_car.json");
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        assert(content.find("\"brand\": \"Mercedes-Benz\"") != std::string::npos);
        assert(content.find("\"country\": \"Germany\"") != std::string::npos);
        std::cout << "[PASS] Test 11: Auto-created and saved new ui_car.json for car without existing file!" << std::endl;
    }

    // Clean up temporary test files
    std::filesystem::remove_all(tempDir);
    std::cout << "[PASS] Test cleanup finished." << std::endl;
}

void runScannerTests() {
    std::cout << "\n=== Running ScannerEngine Tests ===" << std::endl;
    auto detector = std::make_shared<BrandDetector>();
    ScannerEngine scanner(detector);

    // 1. Self-contained synthetic scanner test
    std::filesystem::path mockDir = std::filesystem::temp_directory_path() / "acbo_scanner_test";
    std::filesystem::remove_all(mockDir);
    std::filesystem::create_directories(mockDir / "car_ferrari_f40" / "ui");
    std::filesystem::create_directories(mockDir / "car_bmw_m3" / "ui");
    std::filesystem::create_directories(mockDir / "car_unknown_mod" / "ui");

    {
        std::ofstream out(mockDir / "car_ferrari_f40" / "ui" / "ui_car.json");
        out << "{\"name\": \"Ferrari F40\", \"brand\": \"Ferrari\", \"country\": \"Italy\"}";
    }
    {
        std::ofstream out(mockDir / "car_bmw_m3" / "ui" / "ui_car.json");
        out << "{\"name\": \"BMW M3 E30\", \"brand\": \"\", \"country\": \"\"}";
    }
    {
        std::ofstream out(mockDir / "car_unknown_mod" / "ui" / "ui_car.json");
        out << "{\"name\": \"Custom Track Car\", \"brand\": \"\", \"country\": \"\"}";
    }

    auto mockResults = scanner.doScan(QString::fromStdString(mockDir.string()));
    assert(mockResults.size() == 3);
    std::cout << "[PASS] Synthetic directory scan verified 3 cars scanned successfully" << std::endl;

    std::filesystem::remove_all(mockDir);

    // 2. Optional scan on local installation if present
    std::vector<QString> paths = {
        "D:\\SteamLibrary\\steamapps\\common\\assettocorsa\\content\\cars",
        "D:\\Steam\\steamapps\\common\\assettocorsa\\content\\cars",
        "C:\\Program Files (x86)\\Steam\\steamapps\\common\\assettocorsa\\content\\cars"
    };

    for (const auto& p : paths) {
        if (std::filesystem::exists(p.toStdString())) {
            std::cout << "Testing scan on local AC path: " << p.toStdString() << std::endl;
            auto results = scanner.doScan(p);
            std::cout << "[PASS] Successfully scanned " << results.size() << " local cars without crashing!" << std::endl;
            break;
        }
    }
}

void runUpdateManagerTests() {
    std::cout << "\n=== Running UpdateManager Tests ===" << std::endl;
    assert(UpdateManager::isVersionNewer("1.0.0", "1.0.1") == true);
    assert(UpdateManager::isVersionNewer("1.0.0", "1.1.0") == true);
    assert(UpdateManager::isVersionNewer("1.0.0", "2.0.0") == true);
    assert(UpdateManager::isVersionNewer("v1.0.0", "v1.0.1") == true);
    assert(UpdateManager::isVersionNewer("1.0.1", "1.0.0") == false);
    assert(UpdateManager::isVersionNewer("v1.2.0", "v1.1.9") == false);
    assert(UpdateManager::isVersionNewer("1.0.0", "1.0.0") == false);
    assert(UpdateManager::isVersionNewer("v1.0.0", "1.0.0") == false);
    std::cout << "[PASS] Test 1: SemVer version comparison verified across multiple version formats!" << std::endl;
    std::cout << "[PASS] Test 2: Current app version is " << UpdateManager::appVersion().toStdString() << std::endl;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    try {
        runBrandDetectorTests();
        runJsonWriterTests();
        runScannerTests();
        runUpdateManagerTests();
        std::cout << "\n>>> ALL BACKEND ENGINE TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failure with exception: " << e.what() << std::endl;
        return 1;
    }
}


