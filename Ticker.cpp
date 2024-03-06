#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <curl/curl.h>


#include <algorithm>
// Function to extract a field from a JSON string in order to not depend on a JSON library
std::string get_field_from_json(const std::string& json, const std::string& field) {
    auto pos = json.find(field);
    if (pos != std::string::npos) {
        auto start = json.find(":", pos) + 1;
        auto end = json.find(",", start);
        if (end == std::string::npos) {
            end = json.find("}", start);
        }
        std::string value = json.substr(start, end - start);
        value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
        value.erase(std::remove_if(value.begin(), value.end(), [](char c) { return c == '\"'; }), value.end());
        return value;
    }
    return "0"; 
}
class SymbolTicker {
public:
    static std::atomic<int> active_objects;

    // Constructor
    SymbolTicker(const std::string& sym = "", unsigned int p = 0)
        : symbol(sym), period(p), stop_thread(false), bidPrice("0"), bidQty("0"), askPrice("0"), askQty("0") {
        ++active_objects;
        if (!symbol.empty() && period > 0) {
            fetch_thread = std::thread(&SymbolTicker::fetch_data, this);
        }
    }

    // Copy constructor
    SymbolTicker(const SymbolTicker& other) {
        *this = other;
        ++active_objects;
    }

    // Assignment operator
    SymbolTicker& operator=(const SymbolTicker& other) {
        if (this != &other) {
            symbol = other.symbol;
            period = other.period;
            stop_thread.store(other.stop_thread);
            bidPrice = other.bidPrice;
            bidQty = other.bidQty;
            askPrice = other.askPrice;
            askQty = other.askQty;
        }
        return *this;
    }

    // Move constructor
    SymbolTicker(SymbolTicker&& other) noexcept {
        *this = std::move(other);
        ++active_objects;
    }

    // Destructor
    ~SymbolTicker() {
        stop_thread = true;
        if (fetch_thread.joinable()) {
            fetch_thread.join();
        }
        --active_objects;
    }

    // Fetches data from the Binance API
    void fetch_data() {
        while (!stop_thread) {
            auto now = std::chrono::steady_clock::now();
            if (symbol.empty() || period == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            CURL *curl = curl_easy_init();
            if (curl) {
                std::string readBuffer;
                std::string url = "https://fapi.binance.com/fapi/v1/ticker/bookTicker?symbol=" + symbol;

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
                CURLcode res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                if (res == CURLE_OK) {
                    std::lock_guard<std::mutex> guard(data_mutex);
                    bidPrice = get_field_from_json(readBuffer, "bidPrice");
                    bidQty = get_field_from_json(readBuffer, "bidQty");
                    askPrice = get_field_from_json(readBuffer, "askPrice");
                    askQty = get_field_from_json(readBuffer, "askQty");
                }
            }

            std::this_thread::sleep_until(now + std::chrono::milliseconds(period));
        }
    }

    // Thread-safe read method
    std::tuple<std::string, std::string, std::string, std::string> get_data() {
        std::lock_guard<std::mutex> guard(data_mutex);
        std::string bp = bidPrice;
        std::string bq = bidQty;
        std::string ap = askPrice;
        std::string aq = askQty;

        // Destructive read
        bidPrice = "0";
        bidQty = "0";
        askPrice = "0";
        askQty = "0";

        return {bp, bq, ap, aq};
    }

    // Overload the << operator to print the ticker's data
    friend std::ostream& operator<<(std::ostream& os, SymbolTicker& ticker) {
        auto [bp, bq, ap, aq] = ticker.get_data();
        os << ticker.symbol << "\nbid price: " << bp << "\nask price: " << ap << "\nbid qty: " << bq << "\nask qty: " << aq;
        return os;
    }

    
    void set_symbol(const std::string& sym) {
        std::lock_guard<std::mutex> guard(data_mutex);
        symbol = sym;
    }

    void set_period(unsigned int p) {
        std::lock_guard<std::mutex> guard(data_mutex);
        period = p;
    }

    static int get_obj_count() {
        return active_objects.load();
    }

private:
    std::string symbol;
    unsigned int period;
    std::thread fetch_thread;
    std::atomic<bool> stop_thread;
    std::mutex data_mutex;
    std::string bidPrice, bidQty, askPrice, askQty;

    // libcurl callback function
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
};

std::atomic<int> SymbolTicker::active_objects(0);

int main() {
    // test the SymbolTicker class  
    SymbolTicker btcusdt("SOLUSDT", 1000);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Output the fetched data
    std::cout << btcusdt << std::endl;
    btcusdt.set_symbol("");
    btcusdt.set_period(0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
