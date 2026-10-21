#ifndef CONTRACTS_H
#define CONTRACTS_H

/* Shared contract definitions between order-service and inventory-service.
 * These strings document the exact request/response shapes both services
 * agree to. In a production system these would be generated from a shared
 * schema file in contracts/. */

#define CONTRACT_ORDER_CREATE_REQ \
    "{\"item\": \"string\", \"quantity\": \"int\"}"

#define CONTRACT_ORDER_CREATE_RESP \
    "{\"order_id\": \"string\", \"status\": \"accepted|rejected\", \"total\": \"float\"}"

#define CONTRACT_INVENTORY_CHECK_REQ \
    "{\"item\": \"string\", \"quantity\": \"int\"}"

#define CONTRACT_INVENTORY_CHECK_RESP \
    "{\"available\": \"bool\"}"

#define CONTRACT_INVENTORY_RESERVE_REQ \
    "{\"item\": \"string\", \"quantity\": \"int\", \"order_id\": \"string\"}"

#define CONTRACT_INVENTORY_RESERVE_RESP \
    "{\"reserved\": \"bool\", \"order_id\": \"string\"}"

/* Endpoints */
#define ENDPOINT_ORDER_CREATE   "/order"
#define ENDPOINT_INV_CHECK      "/inventory/check"
#define ENDPOINT_INV_RESERVE    "/inventory/reserve"

#endif
