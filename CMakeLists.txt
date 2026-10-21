cmake_minimum_required(VERSION 3.10)
project(soa_contract_first C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(registry
    src/http_mini.c
    src/registry.c
    src/main.c
)

add_executable(inventory-service
    src/http_mini.c
    src/json_mini.c
    src/inventory_service.c
    src/main.c
)

add_executable(order-service
    src/http_mini.c
    src/json_mini.c
    src/order_service.c
    src/main.c
)

target_include_directories(registry PRIVATE src)
target_include_directories(inventory-service PRIVATE src)
target_include_directories(order-service PRIVATE src)
