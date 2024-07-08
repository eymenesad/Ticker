# Ticker.cpp

## Description
- There should be default constructor, parameterized constructor(s), copy constructor and move constructor.
- Number of active objects of this class should be held and be accessible in some way
- Two main parameters will be given: **symbol** and **period (millisecond)**.
- Default value for symbol parameter is empty string. Default value for period is 0.
- Objects need to fetch data from binance futures book ticker endpoint periodically while symbol is not empty string and period is greater than 0.
- To fetch data, you should send get requests to this REST endpoint: `https://fapi.binance.com/fapi/v1/ticker/bookTicker?symbol=<symbol>`

> For example to get "BTCUSDT" ticker data you need to send get request to:
> https://fapi.binance.com/fapi/v1/ticker/bookTicker?symbol=BTCUSDT
> 
> Example response data:
> {"symbol":"BTCUSDT","bidPrice":"59426.80","bidQty":"6.321","askPrice":"59426.90","askQty":"4.428","time":1709124008356,"lastUpdateId":4057737908956}

- bidPrice, bidQty, askPrice, askQty fields should be saved. These are the data that we want to access.
- Unless one period of time passes, a new get request should not be sent.
- Data getter method/mechanism should perform destructive-read. In other words, data will be erased after a read action.
- Global left-shift operator should be usable with std::cout and objects of the class (look below).
- Periodic rest requests should be performed in a seperate thread.
- Read and write operations must be thread-safe for saved data.

## Usage
1. Compile the Ticker.cpp file using a C++ compiler.
$make Ticker

2. Run the compiled executable.
$./Ticker


