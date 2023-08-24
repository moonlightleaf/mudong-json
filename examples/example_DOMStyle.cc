#include <Document.hpp>

#include <iostream>

int main() {
    json::Document doc;
    auto err = doc.parse("{"
    "    \"precision\": \"zip\","
    "    \"Latitude\": 37.766800000000003,"
    "    \"Longitude\": -122.3959,"
    "    \"Address\": \"\","
    "    \"City\": \"SAN FRANCISCO\","
    "    \"State\": \"CA\","
    "    \"Zip\": \"94107\","
    "    \"Country\": \"US\""
    "    }");

    if (err != json::ParseError::PARSE_OK) {
        std::cerr << json::parseErrorStr(err) << std::endl;
        exit(1);
    }

    //get "Country" field
    //使用operator[](const std::string_view&)必须确保doc树中必须包含"Country"的成员，否则将断言失败
    json::Value& country = doc["Country"];
    std::cout << country.getStringView() << std::endl;
    //更安全的做法是使用Document.findMember(const std::string_view& key)
    json::Value::MemberIterator countryIter = doc.findMember("Country");
    if (countryIter != doc.endMember()) {
        std::cout << countryIter->value.getStringView() << std::endl;
    }

    //set "Address"
    json::Value& addr = doc["Address"];
    addr.setString("Block 1, Street 2");
    std::cout << addr.getStringView() << std::endl;
}