//--------------------------------------------------------------------------
// Copyright (C) 2016-2016 Cisco and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License Version 2 as published
// by the Free Software Foundation.  You may not use, modify or distribute
// this program under any other version of the GNU General Public License.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//--------------------------------------------------------------------------

// host_cache_test.cc author Steve Chew <stechew@cisco.com>
// unit tests for the host cache APIs

#include "host_tracker/host_cache.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

//  Fake AddProtocolReference to avoid bringing in a ton of dependencies.
int16_t AddProtocolReference(const char* protocol)
{
    if (!strcmp("tcp", protocol))
        return 2;
    return 1;
}

TEST_GROUP(host_cache)
{
};

//  Test HostIpKey
TEST(host_cache, host_ip_key_test)
{
    HostIpKey zeroed_hk;
    uint8_t expected_hk[16] =
    { 0xde,0xad,0xbe,0xef,0xab,0xcd,0xef,0x01,0x23,0x34,0x56,0x78,0x90,0xab,0xcd,0xef };

    memset(&zeroed_hk.ip_addr, 0, sizeof(zeroed_hk.ip_addr));

    HostIpKey hkey1;
    CHECK(hkey1 == zeroed_hk);

    HostIpKey hkey2(expected_hk);
    CHECK(hkey2 == expected_hk);
}

//  Test HashHostIpKey
TEST(host_cache, hash_test)
{
    size_t expected_hash_val = 4521729;
    size_t actual_hash_val;
    uint8_t hk[16] =
    { 0x0a,0xff,0x12,0x00,0x00,0x00,0x00,0x00,0x0b,0x00,0x56,0x00,0x00,0x00,0x00,0x00 };

    HashHostIpKey hash_hk;

    actual_hash_val = hash_hk(hk);
    CHECK(actual_hash_val == expected_hash_val);
}

//  Test host_cache_add_host_tracker
TEST(host_cache, host_cache_add_host_tracker_test)
{
    HostTracker* expected_ht = new HostTracker;
    std::shared_ptr<HostTracker> actual_ht;
    uint8_t hk[16] =
    { 0xde,0xad,0xbe,0xef,0xab,0xcd,0xef,0x01,0x23,0x34,0x56,0x78,0x90,0xab,0xcd,0xef };
    sfip_t ip_addr = { 0xde,0xad,0xbe,0xef,0xab,0xcd,0xef,0x01,0x23, };
    sfip_t actual_ip_addr;
    HostIpKey hkey(hk);
    Port port = 443;
    Protocol proto = 6;
    HostApplicationEntry app_entry(proto, port, 2);
    HostApplicationEntry actual_app_entry;
    bool ret;

    expected_ht->set_ip_addr(ip_addr);
    expected_ht->add_service(app_entry);

    host_cache_add_host_tracker(expected_ht);

    ret = host_cache.find(ip_addr.ip8, actual_ht);
    CHECK(true == ret);

    actual_ip_addr = actual_ht->get_ip_addr();
    CHECK(!memcmp(&ip_addr, &actual_ip_addr, sizeof(ip_addr)));

    ret = actual_ht->find_service(proto, port, actual_app_entry);
    CHECK(true == ret);
    CHECK(actual_app_entry == app_entry);

    host_cache.clear();     //  Free HostTracker objects
}

//  Test host_cache_add_service
TEST(host_cache, host_cache_add_service_test)
{
    HostTracker* expected_ht = new HostTracker;
    std::shared_ptr<HostTracker> actual_ht;
    uint8_t hk[16] =
    { 0xde,0xad,0xbe,0xef,0xab,0xcd,0xef,0x01,0x23,0x34,0x56,0x78,0x90,0xab,0xcd,0xef };
    sfip_t ip_addr1 = { 0xde,0xad,0xbe,0xef,0xab,0xcd,0xef,0x01,0x23, };
    sfip_t ip_addr2 = { 0xde,0xad,0xbe,0xef,0xab,0xcd,0xef,0x01,0x23, };
    HostIpKey hkey(hk);
    Port port1 = 443;
    Port port2 = 22;
    Protocol proto1 = 17;
    Protocol proto2 = 6;
    HostApplicationEntry app_entry1(proto1, port1, 1);
    HostApplicationEntry app_entry2(proto2, port2, 2);
    HostApplicationEntry actual_app_entry;
    bool ret;

    //  Initialize cache with a HostTracker.
    host_cache_add_host_tracker(expected_ht);

    //  Add a service to a HostTracker that already exists.
    ret = host_cache_add_service(ip_addr1, proto1, port1, "udp");
    CHECK(true == ret);

    ret = host_cache.find(ip_addr1.ip8, actual_ht);
    CHECK(true == ret);

    ret = actual_ht->find_service(proto1, port1, actual_app_entry);
    CHECK(true == ret);
    CHECK(actual_app_entry == app_entry1);

    //  Try adding service again (should fail since service exists).
    ret = host_cache_add_service(ip_addr1, proto1, port1, "udp");
    CHECK(false == ret);

    //  Add a service with a new IP.
    ret = host_cache_add_service(ip_addr2, proto2, port2, "tcp");
    CHECK(true == ret);

    ret = host_cache.find(ip_addr1.ip8, actual_ht);
    CHECK(true == ret);

    ret = actual_ht->find_service(proto2, port2, actual_app_entry);
    CHECK(true == ret);
    CHECK(actual_app_entry == app_entry2);

    host_cache.clear();     //  Free HostTracker objects
}

int main(int argc, char** argv)
{
    return CommandLineTestRunner::RunAllTests(argc, argv);
}

